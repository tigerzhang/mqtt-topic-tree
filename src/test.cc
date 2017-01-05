//
// Created by parallels on 1/4/17.
//

#include <string>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>

#include "TopicTreeNode.h"

/// make_sure_path make sure a path is existed. If not, create it.
///
/// @param root [in] root node
/// @param topic [in] path
/// \return succ: node of the path; failed: nullptr
Yunba::TopicTreeNode * make_sure_path(Yunba::TopicTreeNode& root, std::string topic);

/// sub subscribe a topic
///
/// \param root [in]
/// \param uid [in]
/// \param topic [in]
/// \return 0: succ, sub already; 1: succ, add a new uid; <0: error
int sub(Yunba::TopicTreeNode& root, uint64_t uid, std::string topic);

void print_uid_route(const Yunba::TopicTreeNode *node);

void print_node_uids(Yunba::TopicTreeNode *node);

Yunba::TopicTreeNode * make_sure_path(Yunba::TopicTreeNode &root, std::string topic) {
    std::vector<std::string> splitVect;
    boost::split(splitVect, topic, boost::is_any_of("/"));

    Yunba::TopicTreeNode* currentNode = &root;
    for (int i=0; i<splitVect.size(); i++) {
        if (i == 0 && splitVect[i] == "")
            continue;

        currentNode = currentNode->addChild(splitVect[i], "").get();
    }

    return currentNode;
}

int sub(Yunba::TopicTreeNode& root, uint64_t uid, std::string topic) {
    auto node = make_sure_path(root, topic);
    return node->addUid(uid);
}

int sub(Yunba::TopicTreeNode& root, uint64_t uid, std::string route_hostname, std::string topic) {
    auto node = make_sure_path(root, topic);
    return node->addUid(uid, route_hostname);
}

int main() {
    Yunba::TopicTreeNode root(nullptr, "root");
    auto appkey1 = root.addChild("appkey1", "");
    auto topic1 = appkey1->addChild("topic1", "");
    auto topic2 = appkey1->addChild("topic2", "");

    auto node = root.findNode("/appkey1");
    std::cout << (node ? node->getData() : "NULL") << std::endl;

    node = root.findNode("/appkey1/topic1");
    std::cout << (node ? node->getData() : "NULL") << std::endl;

    node = root.findNode("/appkey1/topic2");
    std::cout << (node ? node->getData() : "NULL") << std::endl;

    node = root.findNode("/appkey1/topic3");
    std::cout << (node ? node->getData() : "NULL") << std::endl;

    std::cout << "Create /appkey2/topic1" << std::endl;
    int count = 100;

    using namespace std::chrono;
    nanoseconds start_ns = duration_cast< nanoseconds >(
            system_clock::now().time_since_epoch()
    );

    for (int i = 0; i < count; i++)
        sub(root, 2001, "/appkey2/topic1");

    nanoseconds end_ns = duration_cast< nanoseconds >(
            system_clock::now().time_since_epoch());

    auto interval = end_ns - start_ns;
    auto nanoseconds_per_op = interval.count() / count;
    auto ops = 1000 * 1000* 1000 / nanoseconds_per_op;
    std::cout << nanoseconds_per_op << " nanoseconds per op"
              << std::endl
              << ops << " ops"
              << std::endl;


    sub(root, 2002, "/appkey2/topic1");
    node = root.findNode("/appkey2/topic1");
    std::cout << (node ? node->getData() : "NULL") << std::endl;
    print_node_uids(node);

    ///////// test case: sub with route
    sub(root, 2003, "hostname1", "/appkey3/topic2");
    node = root.findNode(("/appkey3/topic2"));

    auto iterpair = node->get_route_index().equal_range("hostname1");
    // check entry
    assert(iterpair.first->first == "hostname1");
    assert(iterpair.first->second == 2003);
    // only one entry
    {
        auto iter = iterpair.first;
        iter++;
        assert(iter == iterpair.second);
    }

    auto uid_to_hostname = node->get_uids_and_route().find(2003);
    assert(uid_to_hostname->second == "hostname1");

    print_uid_route(node);

    ///////// test case: sub with new route
    sub(root, 2003, "hostname2", "/appkey3/topic2");
    node = root.findNode(("/appkey3/topic2"));

    iterpair = node->get_route_index().equal_range("hostname1");
    // item should be replaced
    assert(iterpair.first == iterpair.second);

    iterpair = node->get_route_index().equal_range("hostname2");
    // check entry
    assert(iterpair.first->first == "hostname2");
    assert(iterpair.first->second == 2003);
    // only one entry
    {
        assert(std::distance(iterpair.first, iterpair.second) == 1);
    }

    //////// test case: sub one more uid
    sub(root, 2004, "hostname2", "/appkey3/topic2");
    node = root.findNode(("/appkey3/topic2"));
    iterpair = node->get_route_index().equal_range("hostname2");
    // check entry
    {
        auto i = iterpair.first;
        assert(i->first == "hostname2");
        assert(i->second == 2003);
        i++;
        assert(i->first == "hostname2");
        assert(i->second == 2004);
    }
    // only two entry
    {
        assert(std::distance(iterpair.first, iterpair.second) == 2);
    }

    print_uid_route(node);

    //////// test case: strength test
    for (uint64_t i = 10000; i < 20000; i++) {
        char buff[16];
        snprintf(buff, 16, "hostname%ld", i%10);
        sub(root, i, buff, "/appkey3/topic2");
    }
    node = root.findNode(("/appkey3/topic2"));

    start_ns = duration_cast< nanoseconds >(
            system_clock::now().time_since_epoch()
    );
    iterpair = node->get_route_index().equal_range("hostname3");
    end_ns = duration_cast< nanoseconds >(
            system_clock::now().time_since_epoch()
    );
    interval = end_ns - start_ns;
    nanoseconds_per_op = interval.count() / count;
    std::cout << nanoseconds_per_op << " nanoseconds per op" << std::endl;
    {
        assert(std::distance(iterpair.first, iterpair.second) == 1000);
    }

    return 0;
}

void print_node_uids(Yunba::TopicTreeNode *node) {
    if (node) {
        std::cout << std::endl << "uids of node: " << node->getData() << std::endl;
        for (auto& uid: node->getUids()) {
            std::cout << ": " << uid << std::endl;
        }
    }
}

void print_uid_route(const Yunba::TopicTreeNode *node) {
    if (node) {
        std::cout << std::endl << "uid -> hostname of: " << node->getData() << std::endl;
        for (auto& i: node->get_uids_and_route()) {
            std::cout << ": " << i.first << "->" << i.second << std::endl;
        }
        std::cout << "hostname -> uid of: " << node->getData() << std::endl;
        for (auto& i: node->get_route_index()) {
            std::cout << ": " << i.first << "->" << i.second << std::endl;
        }
    } else {
        std::cout << "Nil node" << std::endl;
    }
}