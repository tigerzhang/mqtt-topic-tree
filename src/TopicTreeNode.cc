//
// Created by parallels on 17-1-4.
//

#include <boost/algorithm/string.hpp>
#include <memory>
#include "TopicTreeNode.h"

namespace Yunba {
    std::map<uint64_t, std::set<TopicTreeNode*>> TopicTreeNode::_uid_subcribed_topic;
    const std::string TopicTreeNode::OFFLINE = "N";

    TopicTreeNode::TopicTreeNode(TopicTreeNode* parent, std::string data) :
    _parent(parent)
    , _name(data)
    {
    }

    std::shared_ptr<TopicTreeNode> TopicTreeNode::addChild(const std::string &name, const std::string &data) {
        auto it = _children.find(name);
        if (it == _children.end()) {
            auto child = std::make_shared<TopicTreeNode>(this, name);
            _children[name] = child;
            return child;
        }

        return it->second;
    }

    void TopicTreeNode::removeChild(const std::string &name) {
        _children.erase(name);
    }

    TopicTreeNode* TopicTreeNode::findNode(const std::string &path) {
        std::vector<std::string> result;
//        boost::trim_if(path, boost::is_any_of("\t ")); // split can't omit separators at ends
        boost::split(result, path, boost::is_any_of("/"), boost::token_compress_on);

        auto current = this;
        for(int i=0; i<result.size(); i++) {
            std::string name = result[i];
            if (i == 0) {
                if (name == "") {
                    name = "/";
                }
            }

            // '/' represents the root node
            if (name == "/") {
                if (_parent != nullptr) {
                    return nullptr;
                } else {
                    continue;
                }
            } else {
                current = current->getChild(name).get();
                if (current == nullptr) {
                    return current;
                } else {
                    if (i == result.size() - 1) {
                        return current;
                    }
                }
            }
        }

        return nullptr;
    }

    std::set<uint64_t> TopicTreeNode::getUids() {
        return _uids;
    }

    const std::map<uint64_t, std::string> &TopicTreeNode::get_uids_and_route() const {
        return _uids_and_route;
    }

    const std::multimap<std::string, uint64_t> &TopicTreeNode::get_route_index() const {
        return _route_index;
    }

    const std::map<uint64_t, std::set<TopicTreeNode *>> &TopicTreeNode::get_uid_subcribed_topic() const {
        return _uid_subcribed_topic;
    }
}