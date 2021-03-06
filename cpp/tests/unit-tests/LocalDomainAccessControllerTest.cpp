/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2017 BMW Car IT GmbH
 * %%
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
 * #L%
 */

#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "libjoynrclustercontroller/access-control/LocalDomainAccessController.h"
#include "libjoynrclustercontroller/access-control/LocalDomainAccessStore.h"

#include "JoynrTest.h"
#include "joynr/Semaphore.h"
#include "joynr/PrivateCopyAssign.h"

#include "tests/utils/MockObjects.h"

using namespace ::testing;
using namespace joynr;
using namespace joynr::infrastructure;
using namespace joynr::infrastructure::DacTypes;

// Consumer permissions are obtained asynchronously
class ConsumerPermissionCallback : public LocalDomainAccessController::IGetConsumerPermissionCallback
{
public:
    ConsumerPermissionCallback() :
        isValid(false),
        permission(Permission::YES),
        sem(0)
    {}

    ~ConsumerPermissionCallback() = default;

    void consumerPermission(Permission::Enum permission) {
        this->permission = permission;
        isValid = true;
        sem.notify();
    }

    void operationNeeded() {
        sem.notify(); // isValid stays false
    }

    bool isPermissionAvailable() const {
        return isValid;
    }

    Permission::Enum getPermission() const {
        return permission;
    }

    // Returns true if the callback was made
    bool expectCallback(int millisecs) {
        return sem.waitFor(std::chrono::milliseconds(millisecs));
    }

private:
    bool isValid;
    Permission::Enum permission;
    Semaphore sem;
};

// Test class
class LocalDomainAccessControllerTest : public testing::TestWithParam<bool> {
public:
    LocalDomainAccessControllerTest() : localDomainAccessStorePtr(nullptr),
                                        localDomainAccessController(),
                                        mockGdacProxyMock(nullptr),
                                        mockGdrcProxy()
    {
    }

    ~LocalDomainAccessControllerTest() override {
        // Delete test specific files
        joynr::test::util::removeFileInCurrentDirectory(".*\\.settings");
        joynr::test::util::removeFileInCurrentDirectory(".*\\.persist");
    }

    void SetUp() override {
        std::unique_ptr<LocalDomainAccessStore> localDomainAccessStore;
        if(GetParam()) {
            // copy access entry file to bin folder for the test so that runtimes will find and load the file
            joynr::test::util::copyTestResourceToCurrentDirectory("AccessStoreTest.persist");

            localDomainAccessStore = std::make_unique<LocalDomainAccessStore>("AccessStoreTest.persist");
        } else {
            localDomainAccessStore = std::make_unique<LocalDomainAccessStore>();
        }
        localDomainAccessStorePtr = localDomainAccessStore.get();
        localDomainAccessController = std::make_unique<LocalDomainAccessController>(std::move(localDomainAccessStore));

        auto mockGdacProxy = std::make_unique<MockGlobalDomainAccessControllerProxy>();
        mockGdacProxyMock = mockGdacProxy.get();
        localDomainAccessController->init(std::move(mockGdacProxy));

        mockGdrcProxy = std::make_shared<MockGlobalDomainRoleControllerProxy>();
        localDomainAccessController->init(mockGdrcProxy);

        userDre = DomainRoleEntry(TEST_USER, DOMAINS, Role::OWNER);
        masterAce = MasterAccessControlEntry(
                TEST_USER,       // uid
                TEST_DOMAIN1,    // domain
                TEST_INTERFACE1, // interface name
                TrustLevel::LOW, // default required trust level
                TRUST_LEVELS,    // possible required trust levels
                TrustLevel::LOW, // default required control entry change trust level
                TRUST_LEVELS,    // possible required control entry change trust levels
                TEST_OPERATION1, // operation
                Permission::NO,  // default comsumer permission
                PERMISSIONS      // possible comsumer permissions
        );
        ownerAce = OwnerAccessControlEntry(
                TEST_USER,       // uid
                TEST_DOMAIN1,    // domain
                TEST_INTERFACE1, // interface name
                TrustLevel::LOW, // required trust level
                TrustLevel::LOW, // required ACE change trust level
                TEST_OPERATION1, // operation
                Permission::YES  // consumer permission
        );
    }

    static const std::string TEST_USER;
    static const std::string TEST_DOMAIN1;
    static const std::string TEST_INTERFACE1;
    static const std::string TEST_OPERATION1;
    static const std::vector<std::string> DOMAINS;
    static const std::vector<Permission::Enum> PERMISSIONS;
    static const std::vector<TrustLevel::Enum> TRUST_LEVELS;
    static const std::string joynrDomain;

protected:
    LocalDomainAccessStore* localDomainAccessStorePtr;
    std::unique_ptr<LocalDomainAccessController> localDomainAccessController;

    MockGlobalDomainAccessControllerProxy* mockGdacProxyMock;
    std::shared_ptr<MockGlobalDomainRoleControllerProxy> mockGdrcProxy;
    OwnerAccessControlEntry ownerAce;
    MasterAccessControlEntry masterAce;
    DomainRoleEntry userDre;
private:
    DISALLOW_COPY_AND_ASSIGN(LocalDomainAccessControllerTest);
};

//----- Constants --------------------------------------------------------------
const std::string LocalDomainAccessControllerTest::TEST_USER("testUser");
const std::string LocalDomainAccessControllerTest::TEST_DOMAIN1("domain1");
const std::string LocalDomainAccessControllerTest::TEST_INTERFACE1("interface1");
const std::string LocalDomainAccessControllerTest::TEST_OPERATION1("operation1");
const std::vector<std::string> LocalDomainAccessControllerTest::DOMAINS = {
        LocalDomainAccessControllerTest::TEST_DOMAIN1
};
const std::vector<Permission::Enum> LocalDomainAccessControllerTest::PERMISSIONS = {
        Permission::NO, Permission::ASK, Permission::YES
};
const std::vector<TrustLevel::Enum> LocalDomainAccessControllerTest::TRUST_LEVELS = {
        TrustLevel::LOW, TrustLevel::MID, TrustLevel::HIGH
};
const std::string LocalDomainAccessControllerTest::joynrDomain("LocalDomainAccessControllerTest.Domain.A");

//----- Tests ------------------------------------------------------------------

TEST_P(LocalDomainAccessControllerTest, testHasRole) {
    localDomainAccessStorePtr->updateDomainRole(userDre);

    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);

    EXPECT_TRUE(localDomainAccessController->hasRole(LocalDomainAccessControllerTest::TEST_USER,
                                                     LocalDomainAccessControllerTest::TEST_DOMAIN1,
                                                     Role::OWNER));
}

TEST_P(LocalDomainAccessControllerTest, consumerPermission) {
    localDomainAccessStorePtr->updateOwnerAccessControlEntry(ownerAce);

    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);
    EXPECT_EQ(
            Permission::YES,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionInvalidOwnerAce) {
    localDomainAccessStorePtr->updateOwnerAccessControlEntry(ownerAce);

    // Update the MasterACE so that it does not permit Permission::YES
    std::vector<Permission::Enum> possiblePermissions = {
            Permission::NO, Permission::ASK
    };
    masterAce.setDefaultConsumerPermission(Permission::ASK);
    masterAce.setPossibleConsumerPermissions(possiblePermissions);
    localDomainAccessStorePtr->updateMasterAccessControlEntry(masterAce);

    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);
    EXPECT_EQ(
            Permission::NO,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionOwnerAceOverrulesMaster) {
    ownerAce.setRequiredTrustLevel(TrustLevel::MID);
    ownerAce.setConsumerPermission(Permission::ASK);
    localDomainAccessStorePtr->updateOwnerAccessControlEntry(ownerAce);
    localDomainAccessStorePtr->updateMasterAccessControlEntry(masterAce);

    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);
    EXPECT_EQ(
            Permission::ASK,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
    EXPECT_EQ(
            Permission::NO,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::LOW
            )
    );
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionOperationWildcard) {
    ownerAce.setOperation(access_control::WILDCARD);
    localDomainAccessStorePtr->updateOwnerAccessControlEntry(ownerAce);

    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);
    EXPECT_EQ(
            Permission::YES,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionAmbigious) {
    // Setup the master with a wildcard operation
    masterAce.setOperation(access_control::WILDCARD);
    std::vector<MasterAccessControlEntry> masterAcesFromGlobalDac;
    masterAcesFromGlobalDac.push_back(masterAce);

    std::vector<OwnerAccessControlEntry> ownerAcesFromGlobalDac;
    ownerAcesFromGlobalDac.push_back(ownerAce);

    // Setup the mock GDAC proxy
    EXPECT_CALL(*mockGdrcProxy, getDomainRolesAsync(_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<1>(std::vector<DomainRoleEntry>()),
                    Return(std::shared_ptr<Future<std::vector<DomainRoleEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMasterAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(masterAcesFromGlobalDac),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMediatorAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(std::vector<MasterAccessControlEntry>()),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getOwnerAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(ownerAcesFromGlobalDac),
                    Return(std::shared_ptr<Future<std::vector<OwnerAccessControlEntry>>>()) // nullptr pointer
            ));

    // Set default return value for Google mock
    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);

    // Get the consumer permission (async)
    auto getConsumerPermissionCallback = std::make_shared<ConsumerPermissionCallback>();

    localDomainAccessController->getConsumerPermission(
            LocalDomainAccessControllerTest::TEST_USER,
            LocalDomainAccessControllerTest::TEST_DOMAIN1,
            LocalDomainAccessControllerTest::TEST_INTERFACE1,
            TrustLevel::HIGH,
            getConsumerPermissionCallback
    );

    EXPECT_TRUE(getConsumerPermissionCallback->expectCallback(1000));

    // The operation is ambigious and interface level permission is not available
    EXPECT_FALSE(getConsumerPermissionCallback->isPermissionAvailable());

    // Operation level permission should work
    EXPECT_EQ(
            Permission::YES,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionCommunicationFailure) {
    // Setup the master with a wildcard operation
    masterAce.setOperation(access_control::WILDCARD);
    std::vector<MasterAccessControlEntry> masterAcesFromGlobalDac;
    masterAcesFromGlobalDac.push_back(masterAce);

    std::vector<OwnerAccessControlEntry> ownerAcesFromGlobalDac;
    ownerAcesFromGlobalDac.push_back(ownerAce);

    // Setup the mock GDAC proxy
    EXPECT_CALL(*mockGdrcProxy, getDomainRolesAsync(_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<1>(std::vector<DomainRoleEntry>()),
                    Return(std::shared_ptr<Future<std::vector<DomainRoleEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMasterAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(masterAcesFromGlobalDac),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMediatorAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<3>(exceptions::JoynrRuntimeException("simulated communication failure")),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getOwnerAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(ownerAcesFromGlobalDac),
                    Return(std::shared_ptr<Future<std::vector<OwnerAccessControlEntry>>>()) // nullptr pointer
            ));

    // Set default return value for Google mock
    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);

    // Get the consumer permission (async)
    auto getConsumerPermissionCallback = std::make_shared<ConsumerPermissionCallback>();

    localDomainAccessController->getConsumerPermission(
            LocalDomainAccessControllerTest::TEST_USER,
            LocalDomainAccessControllerTest::TEST_DOMAIN1,
            LocalDomainAccessControllerTest::TEST_INTERFACE1,
            TrustLevel::HIGH,
            getConsumerPermissionCallback
    );

    EXPECT_TRUE(getConsumerPermissionCallback->expectCallback(1000));
    EXPECT_TRUE(getConsumerPermissionCallback->isPermissionAvailable());
    EXPECT_EQ(Permission::NO, getConsumerPermissionCallback->getPermission());
}

TEST_P(LocalDomainAccessControllerTest, consumerPermissionQueuedRequests) {
    // Setup the master with a wildcard operation
    masterAce.setOperation(access_control::WILDCARD);
    std::vector<MasterAccessControlEntry> masterAcesFromGlobalDac;
    masterAcesFromGlobalDac.push_back(masterAce);

    ownerAce.setOperation(access_control::WILDCARD);
    std::vector<OwnerAccessControlEntry> ownerAcesFromGlobalDac;
    ownerAcesFromGlobalDac.push_back(ownerAce);

    std::function<void(const std::vector<MasterAccessControlEntry>& masterAces)> getMasterAcesOnSuccessFct = [](auto){};

    // Setup the mock GDAC proxy
    EXPECT_CALL(*mockGdrcProxy, getDomainRolesAsync(_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<1>(std::vector<DomainRoleEntry>()),
                    Return(std::shared_ptr<Future<std::vector<DomainRoleEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMasterAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    SaveArg<2>(&getMasterAcesOnSuccessFct),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getMediatorAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(std::vector<MasterAccessControlEntry>()),
                    Return(std::shared_ptr<Future<std::vector<MasterAccessControlEntry>>>()) // nullptr pointer
            ));
    EXPECT_CALL(*mockGdacProxyMock, getOwnerAccessControlEntriesAsync(_,_,_,_))
            .Times(1)
            .WillOnce(DoAll(
                    InvokeArgument<2>(ownerAcesFromGlobalDac),
                    Return(std::shared_ptr<Future<std::vector<OwnerAccessControlEntry>>>()) // nullptr pointer
            ));

    // Expect a call to subscribe for the not present Entry
    EXPECT_CALL(*mockGdacProxyMock, subscribeToMasterAccessControlEntryChangedBroadcast(_,_,_)).    Times(1);
    EXPECT_CALL(*mockGdacProxyMock, subscribeToMediatorAccessControlEntryChangedBroadcast(_,_,_)).  Times(1);
    EXPECT_CALL(*mockGdacProxyMock, subscribeToOwnerAccessControlEntryChangedBroadcast(_,_,_)).     Times(1);

    // Set default return value for Google mock
    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);

    // Get the consumer permission (async)
    auto getConsumerPermissionCallback1 = std::make_shared<ConsumerPermissionCallback>();

    localDomainAccessController->getConsumerPermission(
            LocalDomainAccessControllerTest::TEST_USER,
            LocalDomainAccessControllerTest::TEST_DOMAIN1,
            LocalDomainAccessControllerTest::TEST_INTERFACE1,
            TrustLevel::HIGH,
            getConsumerPermissionCallback1
    );

    // Make another request for consumer permission
    auto getConsumerPermissionCallback2 = std::make_shared<ConsumerPermissionCallback>();

    localDomainAccessController->getConsumerPermission(
                LocalDomainAccessControllerTest::TEST_USER,
                LocalDomainAccessControllerTest::TEST_DOMAIN1,
                LocalDomainAccessControllerTest::TEST_INTERFACE1,
                TrustLevel::HIGH,
                getConsumerPermissionCallback2
    );

    EXPECT_FALSE(getConsumerPermissionCallback1->expectCallback(0));
    EXPECT_FALSE(getConsumerPermissionCallback2->expectCallback(0));

    // Provide the missing response to the LocalDomainAccessController
    getMasterAcesOnSuccessFct(masterAcesFromGlobalDac);

    EXPECT_TRUE(getConsumerPermissionCallback1->isPermissionAvailable());
    EXPECT_TRUE(getConsumerPermissionCallback2->isPermissionAvailable());
    EXPECT_EQ(Permission::YES, getConsumerPermissionCallback1->getPermission());
    EXPECT_EQ(Permission::YES, getConsumerPermissionCallback2->getPermission());

}

// true: with persist file
// false: without
INSTANTIATE_TEST_CASE_P(WithOrWithoutPersistFile,
    LocalDomainAccessControllerTest,
    Bool()
);

TEST(LocalDomainAccessControllerPeristedTest, persistedAcesAreUsed) {
    auto mockGdacProxyPtr = std::make_unique<MockGlobalDomainAccessControllerProxy>();
    auto mockGdacProxy = mockGdacProxyPtr.get();

    // Do not contact GDAC (do not perform any get* operation) for persisted ACEs
    EXPECT_CALL(*mockGdacProxy, getMasterAccessControlEntriesAsync(_,_,_,_))    .Times(0);
    EXPECT_CALL(*mockGdacProxy, getMediatorAccessControlEntriesAsync(_,_,_,_))  .Times(0);
    EXPECT_CALL(*mockGdacProxy, getOwnerAccessControlEntriesAsync(_,_,_,_))     .Times(0);

    // Expect only calls to subscribeTo methods
    EXPECT_CALL(*mockGdacProxy, subscribeToMasterAccessControlEntryChangedBroadcast(_,_,_)).    Times(1);
    EXPECT_CALL(*mockGdacProxy, subscribeToMediatorAccessControlEntryChangedBroadcast(_,_,_)).  Times(1);
    EXPECT_CALL(*mockGdacProxy, subscribeToOwnerAccessControlEntryChangedBroadcast(_,_,_)).     Times(1);

    // Copy access entry file to bin folder for the test so that runtimes will find and load the file
    joynr::test::util::copyTestResourceToCurrentDirectory("AccessStoreTest.persist");

    // Load persisted ACEs
    auto localDomainAccessStore = std::make_unique<LocalDomainAccessStore>("AccessStoreTest.persist");
    auto localDomainAccessController =
            std::make_unique<LocalDomainAccessController>(std::move(localDomainAccessStore));

    localDomainAccessController->init(std::move(mockGdacProxyPtr));

    // Set default return value for Google mock
    std::string defaultString;
    DefaultValue<std::string>::Set(defaultString);

    EXPECT_EQ(
            Permission::NO,
            localDomainAccessController->getConsumerPermission(
                    LocalDomainAccessControllerTest::TEST_USER,
                    LocalDomainAccessControllerTest::TEST_DOMAIN1,
                    LocalDomainAccessControllerTest::TEST_INTERFACE1,
                    LocalDomainAccessControllerTest::TEST_OPERATION1,
                    TrustLevel::HIGH
            )
    );
}
