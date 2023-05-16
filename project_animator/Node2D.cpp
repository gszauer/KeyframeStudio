#include "Node2D.h"

void Animation::ReleaseMemory() {
	if (name != 0) {
		MemRelease(name);
		name = 0;
	}

	if (tracks != 0) {
		Track* track = tracks;
		while (track != 0) {
			Track* to_delete_track = track;
			if (to_delete_track->frames != 0) {
				MemRelease(to_delete_track->frames);
				to_delete_track->frames = 0;
				to_delete_track->frameCount = 0;
			}
			MemRelease(to_delete_track);
			track = track->next;
		}
		tracks = 0;
		numTracks = 0;
	}
}

u32 CountDescendants(Node2D* node) {
	if (node == 0) {
		return 0;
	}

	u32 result = 0;

	Node2D* itr = node;
	bool traversing = true;
	while (traversing) {
		if (itr != node) { // Process
			result += 1;
		}

		if (itr->firstChild) {
			itr = itr->firstChild;
		}
		else {
			while (itr->next == 0) {
				if (itr == node) {
					traversing = false;
					break;
				}
				itr = itr->parent;
			}
			if (itr == node) {
				traversing = false;
				break;
			}
			itr = itr->next;
		}
	}

	return result;
}

void ForEachNode(Node2D* root, NodeVisitor callback, void* userData) {
	Node2D* itr = root;
	bool traversing = true;
	while (traversing) {
		callback(itr, userData);

		if (itr->firstChild) {
			itr = itr->firstChild;
		}
		else {
			while (itr->next == 0) {
				if (itr == root) {
					traversing = false;
					break;
				}
				itr = itr->parent;
			}
			if (itr == root) { 
				traversing = false;
				break;
			}
			itr = itr->next;
		}
	}
}

void RecursivleyDestroyNode(Node2D* node) {
	// Unhook from hierarchy
	if (node->parent != 0) {
		RemoveChild(*node->parent, *node);
	}

	// Iterate subtree and release all resources

	Node2D* itr = node;
	bool traversing = true;
	while (traversing) {
		if (itr->firstChild) {
			itr = itr->firstChild;
		}
		else {
			// At the bottom of the tree. This node has no children. 
			Node2D* parent = itr->parent;
			Node2D* next = itr->next;

			if (parent != 0) {
				RemoveChild(*parent, *itr);
			}
			PlatformAssert(itr->parent == 0, __LOCATION__);
			PlatformAssert(itr->firstChild == 0, __LOCATION__);

			{ // Release memory
				if (itr->name != 0) {
					MemRelease(itr->name);
					itr->name = 0;
				}

				if (itr == node) {
					traversing = false;
				}
				MemRelease(itr);
			}

			if (next != 0) {
				itr = next;
			}
			else {
				itr = parent;
			}
		}
	}
}

Node2D* GetLastChild(Node2D& node) {
	if (node.firstChild == 0) {
		return 0;
	}
	Node2D* iter = node.firstChild;
	while (iter->next != 0) {
		iter = iter->next;
	}
	return iter;
}

Node2D* GetChildById(Node2D& node, u32 uid) {
	if (node.firstChild == 0 || uid == 0) {
		return 0;
	}

	Node2D* iter = node.firstChild;
	while (iter->next != 0) {
		if (iter->uid == uid) {
			break;
		}
		iter = iter->next;
	}

	return iter;
}

bool RemoveChild(Node2D& parent, Node2D& child) {
	if (parent.firstChild == 0) {
		return false;
	}

	Node2D* iter = parent.firstChild;
	Node2D* prev = 0;
	while (iter != 0) {
		if (iter == &child) {
			break;
		}
		prev = iter;
		iter = iter->next;
	}

	if (iter == 0) {
		return false;
	}

	if (prev != 0) {
		prev->next = iter->next;
	}
	else {
		parent.firstChild = iter->next;
	}

	child.parent = 0;
	child.depth = 0;
	child.next = 0;

	return true;
}

bool AddChild(Node2D& parent, Node2D& child) {
	Node2D* lastChild = GetLastChild(parent);

	if (lastChild != 0) {
		lastChild->next = &child;
		child.next = 0;
	}
	else {
		child.next = parent.firstChild;
		parent.firstChild = &child;
	}
	child.parent = &parent;
	child.depth = parent.depth + 1;

	return true;
}

bool AddChild(Node2D& parent, Node2D& child, u32 lastSibling) {
	Node2D* prevNode = GetChildById(parent, lastSibling);

	if (prevNode != 0) {
		child.next = prevNode->next;
		prevNode->next = &child;
	}
	else {
		child.next = parent.firstChild;
		parent.firstChild = &child;
	}
	child.parent = &parent;
	child.depth = parent.depth + 1;

	return true;
}

void SetParent(Node2D* parent, Node2D& child) {
	if (child.parent != 0) {
		bool removed = RemoveChild(*child.parent, child);
		PlatformAssert(removed, __LOCATION__);
	}

	child.parent = parent;
	child.depth = 0;
	if (parent != 0) {
		child.depth = parent->depth + 1;
	}

	if (parent != 0) {
		AddChild(*child.parent, child);

		u32 parentDepth = 0;
		Node2D* iter = parent;
		while (iter != 0) {
			parentDepth += 1;
			iter = iter->parent;
		}
		parent->depth = parentDepth == 0? 0 : parentDepth - 1;
		ForEachNode(parent, [](Node2D* node, void* userData) {
			if (node != userData) {
				node->depth = node->parent->depth + 1;
			}
		}, parent);
	}
}

void SetParent(Node2D* parent, Node2D& child, u32 lastSibling) {
	if (child.parent != 0) {
		bool removed = RemoveChild(*child.parent, child);
		PlatformAssert(removed, __LOCATION__);
	}

	child.parent = parent;
	child.depth = 0;

	if (parent != 0) {
		AddChild(*child.parent, child, lastSibling);
		child.depth = parent->depth + 1;

		u32 parentDepth = 0;
		Node2D* iter = parent;
		while (iter != 0) {
			parentDepth += 1;
			iter = iter->parent;
		}
		parent->depth = parentDepth == 0 ? 0 : parentDepth - 1;
		ForEachNode(parent, [](Node2D* node, void* userData) {
			if (node != userData) {
				node->depth = node->parent->depth + 1;
			}
		}, parent);
	}
}

void SetName(Node2D& node, const char* name) {
	u32 len = 0;
	if (name != 0) {
		for (len = 0; name[len] != 0; ++len);
	}

	if (len > node.nameCapacity) {
		while (len > node.nameCapacity) {
			node.nameCapacity = node.nameCapacity * 2;
		}
		MemRelease(node.name);
		node.name = (char*)MemAlloc(node.nameCapacity + 1);
		MemClear(node.name, node.nameCapacity + 1);
		node.nameLength = 0;
	}

	for (u32 i = 0; i < len; ++i) {
		node.name[i] = name[i];
	}
	node.name[len] = 0;
	node.nameLength = len;
}