#include "pch.h"
#include "Node.h"
#include "Action.h"
#include <iostream>


	int Node::last_id_ = 0;

	Node::Node() : g_(0), h_(0)
	{
		id_ = ++last_id_;
	}

	Node::Node(const WorldState state, int g, int h, int parent_id, Action* action) :
		ws_(state), parent_id_(parent_id), g_(g), h_(h), action_(action)
	{
		id_ = ++last_id_;
	}

	bool operator<(const Node& lhs, const Node& rhs)
	{
		return lhs.f() < rhs.f();
	}

	//bool goap::Node::operator<(const Node& other) {
	//    return f() < other.f();
	//}
