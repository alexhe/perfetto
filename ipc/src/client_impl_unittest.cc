/*
 * Copyright (C) 2017 The Android Open foo Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc/src/client_impl.h"

#include <string>

#include "base/test/test_task_runner.h"
#include "base/utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ipc/service_descriptor.h"
#include "ipc/service_proxy.h"
#include "ipc/src/buffered_frame_deserializer.h"
#include "ipc/src/unix_socket.h"

#include "client_unittest_messages.pb.h"

namespace perfetto {
namespace ipc {
namespace {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Mock;

constexpr char kSockName[] = "/tmp/perfetto_client_impl_unittest.sock";

// A fake ServiceProxy. This fakes the client-side class that would be
// auto-generated from .proto-files.
class FakeProxy : public ServiceProxy {
 public:
  FakeProxy(const char* service_name, ServiceProxy::EventListener* el)
      : ServiceProxy(el), service_name_(service_name) {}

  const ServiceDescriptor& GetDescriptor() override {
    auto reply_decoder = [](const std::string& proto) {
      std::unique_ptr<ProtoMessage> reply(new ReplyProto());
      EXPECT_TRUE(reply->ParseFromString(proto));
      return reply;
    };
    if (!descriptor_.service_name) {
      descriptor_.service_name = service_name_;
      descriptor_.methods.push_back({"FakeMethod1", nullptr, reply_decoder});
    }
    return descriptor_;
  }

  const char* service_name_;
  ServiceDescriptor descriptor_;
};

class MockEventListener : public ServiceProxy::EventListener {
 public:
  MOCK_METHOD0(OnConnect, void());
  MOCK_METHOD0(OnDisconnect, void());
};

// A fake host implementation. Listens on |kSockName| and replies to IPC
// metohds like a real one.
class FakeHost : public UnixSocket::EventListener {
 public:
  struct FakeMethod {
    MethodID id;
    MOCK_METHOD2(OnInvoke,
                 void(const Frame::InvokeMethod&, Frame::InvokeMethodReply*));
  };  // FakeMethod.

  struct FakeService {
    FakeMethod* AddFakeMethod(const std::string& name) {
      auto it_and_inserted =
          methods.emplace(name, std::unique_ptr<FakeMethod>(new FakeMethod()));
      EXPECT_TRUE(it_and_inserted.second);
      FakeMethod* method = it_and_inserted.first->second.get();
      method->id = ++last_method_id;
      return method;
    }

    ServiceID id;
    std::map<std::string, std::unique_ptr<FakeMethod>> methods;
    MethodID last_method_id = 0;
  };  // FakeService.

  explicit FakeHost(base::TaskRunner* task_runner) {
    unlink(kSockName);
    listening_sock = UnixSocket::Listen(kSockName, this, task_runner);
    EXPECT_TRUE(listening_sock->is_listening());
  }
  ~FakeHost() override { unlink(kSockName); }

  FakeService* AddFakeService(const std::string& name) {
    auto it_and_inserted =
        services.emplace(name, std::unique_ptr<FakeService>(new FakeService()));
    EXPECT_TRUE(it_and_inserted.second);
    FakeService* svc = it_and_inserted.first->second.get();
    svc->id = ++last_service_id;
    return svc;
  }

  // UnixSocket::EventListener implementation.
  void OnNewIncomingConnection(
      UnixSocket*,
      std::unique_ptr<UnixSocket> new_connection) override {
    ASSERT_FALSE(client_sock);
    client_sock = std::move(new_connection);
  }

  void OnDataAvailable(UnixSocket* sock) override {
    if (sock != client_sock.get())
      return;
    auto buf = frame_deserializer.BeginReceive();
    size_t rsize = client_sock->Receive(buf.data, buf.size);
    EXPECT_TRUE(frame_deserializer.EndReceive(rsize));
    while (std::unique_ptr<Frame> frame = frame_deserializer.PopNextFrame())
      OnFrameReceived(*frame);
  }

  void OnFrameReceived(const Frame& req) {
    if (req.msg_case() == Frame::kMsgBindService) {
      auto svc_it = services.find(req.msg_bind_service().service_name());
      ASSERT_NE(services.end(), svc_it);
      const FakeService& svc = *svc_it->second.get();
      Frame reply;
      reply.set_request_id(req.request_id());
      reply.mutable_msg_bind_service_reply()->set_success(true);
      reply.mutable_msg_bind_service_reply()->set_service_id(svc.id);
      for (const auto& method_it : svc.methods) {
        auto* method = reply.mutable_msg_bind_service_reply()->add_methods();
        method->set_name(method_it.first);
        method->set_id(method_it.second->id);
      }
      return Reply(reply);
    } else if (req.msg_case() == Frame::kMsgInvokeMethod) {
      // Lookup the service and method.
      Frame reply;
      reply.set_request_id(req.request_id());
      for (const auto& svc : services) {
        if (svc.second->id != req.msg_invoke_method().service_id())
          continue;
        for (const auto& method : svc.second->methods) {
          if (method.second->id != req.msg_invoke_method().method_id())
            continue;
          method.second->OnInvoke(req.msg_invoke_method(),
                                  reply.mutable_msg_invoke_method_reply());
        }
      }
      // If either the method or the service are not found, |success| will be
      // false by default.
      return Reply(reply);

    } else {
      FAIL() << "Unknown request";
    }
  }

  void Reply(const Frame& frame) {
    auto buf = BufferedFrameDeserializer::Serialize(frame);
    ASSERT_TRUE(client_sock->is_connected());
    EXPECT_TRUE(client_sock->Send(buf.data(), buf.size()));
  }

  BufferedFrameDeserializer frame_deserializer;
  std::unique_ptr<UnixSocket> listening_sock;
  std::unique_ptr<UnixSocket> client_sock;
  std::map<std::string, std::unique_ptr<FakeService>> services;
  ServiceID last_service_id = 0;
};  // FakeHost.

class ClientImplTest : public ::testing::Test {
 public:
  void SetUp() override {
    task_runner_.reset(new base::TestTaskRunner());
    host_.reset(new FakeHost(task_runner_.get()));
    cli_ = Client::CreateInstance(kSockName, task_runner_.get());
  }

  void TearDown() override {
    cli_.reset();
    host_.reset();
    task_runner_->RunUntilIdle();
    task_runner_.reset();
  }

  ::testing::StrictMock<MockEventListener> proxy_events_;
  std::unique_ptr<base::TestTaskRunner> task_runner_;
  std::unique_ptr<FakeHost> host_;
  std::unique_ptr<Client> cli_;
};

TEST_F(ClientImplTest, BindAndInvokeMethod) {
  auto* host_svc = host_->AddFakeService("FakeSvc");
  auto* host_method = host_svc->AddFakeMethod("FakeMethod1");

  std::unique_ptr<FakeProxy> proxy(new FakeProxy("FakeSvc", &proxy_events_));

  // Bind |proxy| to the fake host.
  cli_->BindService(proxy->GetWeakPtr());
  auto on_connect = task_runner_->CreateCheckpoint("on_connect");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect));
  task_runner_->RunUntilCheckpoint("on_connect");

  // Invoke a valid method.
  EXPECT_CALL(*host_method, OnInvoke(_, _))
      .WillOnce(Invoke(
          [](const Frame::InvokeMethod& req, Frame::InvokeMethodReply* reply) {
            RequestProto req_args;
            EXPECT_TRUE(req_args.ParseFromString(req.args_proto()));
            EXPECT_EQ("req_data", req_args.data());
            ReplyProto reply_args;
            reply->set_reply_proto(reply_args.SerializeAsString());
            reply->set_success(true);
          }));

  RequestProto req;
  req.set_data("req_data");
  auto on_invoke_reply = task_runner_->CreateCheckpoint("on_invoke_reply");
  DeferredBase deferred_reply(
      [on_invoke_reply](AsyncResult<ProtoMessage> reply) {
        EXPECT_TRUE(reply.success());
        on_invoke_reply();
      });
  proxy->BeginInvoke("FakeMethod1", req, std::move(deferred_reply));
  task_runner_->RunUntilCheckpoint("on_invoke_reply");

  // Invoke an invalid method.
  auto on_invalid_invoke = task_runner_->CreateCheckpoint("on_invalid_invoke");
  DeferredBase deferred_reply2(
      [on_invalid_invoke](AsyncResult<ProtoMessage> reply) {
        EXPECT_FALSE(reply.success());
        on_invalid_invoke();
      });
  RequestProto empty_req;
  proxy->BeginInvoke("InvaidMethod", empty_req, std::move(deferred_reply2));
  task_runner_->RunUntilCheckpoint("on_invalid_invoke");
}

TEST_F(ClientImplTest, BindSameServiceMultipleTimesShouldFail) {
  host_->AddFakeService("FakeSvc");

  std::unique_ptr<FakeProxy> proxy[3];
  for (size_t i = 0; i < base::ArraySize(proxy); i++)
    proxy[i].reset(new FakeProxy("FakeSvc", &proxy_events_));

  // Bind to the host.
  for (size_t i = 0; i < base::ArraySize(proxy); i++) {
    auto checkpoint_name = "on_connect_or_disconnect" + std::to_string(i);
    auto closure = task_runner_->CreateCheckpoint(checkpoint_name);
    if (i == 0) {
      // Only the first call should succeed.
      EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(closure));
    } else {
      EXPECT_CALL(proxy_events_, OnDisconnect()).WillOnce(Invoke(closure));
    }
    cli_->BindService(proxy[i]->GetWeakPtr());
    task_runner_->RunUntilCheckpoint(checkpoint_name);
  }
}

TEST_F(ClientImplTest, BindRequestsAreQueuedIfNotConnected) {
  host_->AddFakeService("FakeSvc1");
  host_->AddFakeService("FakeSvc2");

  std::unique_ptr<FakeProxy> proxy1(new FakeProxy("FakeSvc1", &proxy_events_));
  std::unique_ptr<FakeProxy> proxy2(new FakeProxy("FakeSvc2", &proxy_events_));

  // Bind the services (in opposite order of creation) before running any task.
  cli_->BindService(proxy2->GetWeakPtr());
  cli_->BindService(proxy1->GetWeakPtr());

  InSequence seq;
  auto on_connect1 = task_runner_->CreateCheckpoint("on_connect1");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect1));

  auto on_connect2 = task_runner_->CreateCheckpoint("on_connect2");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect2));

  task_runner_->RunUntilCheckpoint("on_connect1");
  task_runner_->RunUntilCheckpoint("on_connect2");
}

// The deferred callbacks for both binding a service and invoking a method
// should be dropped if the ServiceProxy object is destroyed prematurely.
TEST_F(ClientImplTest, DropCallbacksIfServiceProxyIsDestroyed) {
  auto* host_svc = host_->AddFakeService("FakeSvc");
  auto* host_method = host_svc->AddFakeMethod("FakeMethod1");

  std::unique_ptr<FakeProxy> proxy(new FakeProxy("FakeSvc", &proxy_events_));

  // First bind the service but destroy it before ClientImpl manages to run any
  // tasks. No OnConnect() should be called.
  cli_->BindService(proxy->GetWeakPtr());
  proxy.reset();
  task_runner_->RunUntilIdle();
  ASSERT_TRUE(Mock::VerifyAndClearExpectations(&proxy_events_));

  // Now bind it successfully, invoke a method but destroy the proxy before
  // the method reply is dispatched. The DeferredReply should be rejected,
  // despite the fact that the host gave a successful reply.
  proxy.reset(new FakeProxy("FakeSvc", &proxy_events_));
  auto on_connect = task_runner_->CreateCheckpoint("on_connect");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect));
  cli_->BindService(proxy->GetWeakPtr());
  task_runner_->RunUntilCheckpoint("on_connect");

  RequestProto req;
  auto on_reply_sent = task_runner_->CreateCheckpoint("on_reply_sent");
  EXPECT_CALL(*host_method, OnInvoke(_, _))
      .WillOnce(Invoke([on_reply_sent](const Frame::InvokeMethod&,
                                       Frame::InvokeMethodReply* reply) {
        ReplyProto reply_args;
        reply->set_success(true);
        on_reply_sent();
      }));

  auto on_reject = task_runner_->CreateCheckpoint("on_reject");
  DeferredBase deferred_reply([on_reject](AsyncResult<ProtoMessage> res) {
    ASSERT_FALSE(res.success());
    on_reject();
  });
  proxy->BeginInvoke("FakeMethod1", req, std::move(deferred_reply));
  proxy.reset();
  task_runner_->RunUntilCheckpoint("on_reject");
  task_runner_->RunUntilCheckpoint("on_reply_sent");
}

// If the Client object is destroyed before the ServiceProxy, the ServiceProxy
// should see a Disconnect() call and any pending callback should be rejected.
TEST_F(ClientImplTest, ClientDestroyedBeforeProxy) {
  auto* host_svc = host_->AddFakeService("FakeSvc");
  host_svc->AddFakeMethod("FakeMethod1");

  std::unique_ptr<FakeProxy> proxy(new FakeProxy("FakeSvc", &proxy_events_));
  auto on_connect = task_runner_->CreateCheckpoint("on_connect");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect));
  cli_->BindService(proxy->GetWeakPtr());
  task_runner_->RunUntilCheckpoint("on_connect");

  auto on_reject = task_runner_->CreateCheckpoint("on_reject");
  DeferredBase deferred_reply([on_reject](AsyncResult<ProtoMessage> res) {
    ASSERT_FALSE(res.success());
    on_reject();
  });
  RequestProto req;
  proxy->BeginInvoke("FakeMethod1", req, std::move(deferred_reply));
  EXPECT_CALL(proxy_events_, OnDisconnect());
  cli_.reset();
  task_runner_->RunUntilCheckpoint("on_reject");
}

// Test that OnDisconnect() is invoked if the host is not reachable.
TEST_F(ClientImplTest, HostNotReachable) {
  host_.reset();

  std::unique_ptr<FakeProxy> proxy(new FakeProxy("FakeSvc", &proxy_events_));

  auto on_disconnect = task_runner_->CreateCheckpoint("on_disconnect");
  EXPECT_CALL(proxy_events_, OnDisconnect()).WillOnce(Invoke(on_disconnect));
  cli_->BindService(proxy->GetWeakPtr());
  task_runner_->RunUntilCheckpoint("on_disconnect");
}

// Test that OnDisconnect() is invoked if the host shuts down prematurely.
TEST_F(ClientImplTest, HostDisconnection) {
  host_->AddFakeService("FakeSvc");

  std::unique_ptr<FakeProxy> proxy(new FakeProxy("FakeSvc", &proxy_events_));

  // Bind |proxy| to the fake host.
  cli_->BindService(proxy->GetWeakPtr());
  auto on_connect = task_runner_->CreateCheckpoint("on_connect");
  EXPECT_CALL(proxy_events_, OnConnect()).WillOnce(Invoke(on_connect));
  task_runner_->RunUntilCheckpoint("on_connect");

  auto on_disconnect = task_runner_->CreateCheckpoint("on_disconnect");
  EXPECT_CALL(proxy_events_, OnDisconnect()).WillOnce(Invoke(on_disconnect));
  host_.reset();
  task_runner_->RunUntilCheckpoint("on_disconnect");
}

// TODO(primiano): add the tests below.
// TEST(ClientImplTest, BindAndInvokeStreamingMethod) {}
// TEST(ClientImplTest, UnparsableReply) {}

}  // namespace
}  // namespace ipc
}  // namespace perfetto
