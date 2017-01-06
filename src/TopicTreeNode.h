#ifndef _TOPIC_TREE_H_
#define _TOPIC_TREE_H_

#include <cstddef>
#include <map>
#include <set>
#include <memory>

namespace Yunba {

class TopicTreeNode {

//        using TopicTreeNodePtr = std::shared_ptr<TopicTreeNode<T>>;
//        typedef std::shared_ptr<TopicTreeNode<T>> TopicTreeNodePtr;
private:
    std::string _name;
    TopicTreeNode *_parent;
    std::map<std::string, std::shared_ptr<TopicTreeNode> > _children;
    std::set<uint64_t> _uids;
    std::map<uint64_t, std::string> _uids_and_route;
    std::multimap<std::string, uint64_t> _route_index;
    static std::map<uint64_t, std::set<TopicTreeNode *>> _uid_subcribed_topic;
public:
    const std::map<uint64_t, std::set<TopicTreeNode *>> &get_uid_subcribed_topic() const;

public:
    const std::map<uint64_t, std::string> &get_uids_and_route() const;

    const std::multimap<std::string, uint64_t> &get_route_index() const;

public:
    static const std::string OFFLINE;

    TopicTreeNode(TopicTreeNode *parent, std::string data);

    ~TopicTreeNode() {};

    std::string getName() const {
        return _name;
    };

    void setData(const std::string &data) {
        _name = data;
    };

    std::shared_ptr<TopicTreeNode> addChild(const std::string &name, const std::string &data);

    void removeChild(const std::string &name);

    std::shared_ptr<TopicTreeNode> findChild(const std::string &data) const {};

    std::shared_ptr<TopicTreeNode> getChild(const std::string &name) const {
        std::shared_ptr<TopicTreeNode> p;
        try {
            p = _children.at(name);
        } catch (const std::out_of_range &oor) {
            p = nullptr;
        }
        return p;
    };

    TopicTreeNode *getParent() const { return _parent; };

    int getNumChildren() const {};

    TopicTreeNode *findNode(const std::string &path);

    int addUid(uint64_t uid, std::string route) {

        bool remove_old = false;
        auto uid_to_route = _uids_and_route.find(uid);
        if (uid_to_route != _uids_and_route.end()) {
            if (uid_to_route->second != route) {
                remove_old = true;
            }
        }

        if (remove_old) {
            std::string &current_route = uid_to_route->second;
            // remove current <route, uid> pair
            auto iterpair = _route_index.equal_range(current_route);
            for (auto it = iterpair.first; it != iterpair.second; it++) {
                if (it->second == uid) {
                    _route_index.erase(it);
                }
            }
        }

        _route_index.insert(std::pair<std::string, uint64_t>(route, uid));
        _uids_and_route[uid] = route;

        // update id -> topics
        auto uid_to_topics = _uid_subcribed_topic.find(uid);
        if (uid_to_topics == _uid_subcribed_topic.end()) {
            _uid_subcribed_topic[uid] = std::set<TopicTreeNode *>();
        }
        uid_to_topics = _uid_subcribed_topic.find(uid);
        uid_to_topics->second.insert(this);
    }

    int addUid(uint64_t uid) {
        _uids.insert(uid);

        addUid(uid, OFFLINE);
    }

    std::set<uint64_t> getUids();
};

}

#endif