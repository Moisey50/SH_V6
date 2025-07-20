#include "stdafx.h"
#include "NodeList.h"

const float Definitions::PositionTolerance = 0.1f;

Node::Node()
{
	spot = NULL;
	next = previous = NULL;
}
Node::Node(Spot* pSpot)
{
	spot = pSpot;
	next = previous = NULL;
}

NodeList::NodeList()
{
	head = tail = NULL;
	cnt = 0;
}
NodeList::~NodeList()
{
	delAll();
}


void NodeList::move(Node* node, Node* previous)
{
	if (node->previous)
		node->previous->next = node->next;
	else
		head = node->next;

	if(node->next)
		node->next->previous = node->previous;
	else
		tail = node->previous;


	node->next = previous->next;
	previous->next = node;

	node->previous = previous;

	if(node->next) 
		node->next->previous = node;
	else
		tail = node;
}
Node* NodeList::insert(Spot* spot)
{
	if(!spot)
		return NULL;

	Node* node = new Node(spot);

	insert(node, &head, &tail, &cnt);
	return node;
}
bool NodeList::remove(Node* node)
{
	remove(node, &head, &tail, &cnt);

	return true;
}
void NodeList::del(Node* node)
{
	remove(node);

	delete node;
}
void NodeList::delAll()
{
	Node* node = head;
	Node* next;

	while(node)
	{
		next = node->next;
		del(node);
		node = next;
	}
}
Node* NodeList::getNearestNeighbor(const Spot* spot) const
{
	if(cnt<1 || !spot)
		return NULL;

	Node* next;
	Node* candidate = NULL;
	double d, min;

	next = head;

	min = DBL_MAX;

	while (next)
	{
		if(next->spot == spot)
		{
			next = next->next;
			continue;				
		}

		d = spot->getDistance(*next->spot);

		if(d < min)
		{
			min = d;
			candidate = next;
		}

		next = next->next;
	}

	return candidate;
}
bool NodeList::sort(Node* edge)
{
	Node* t = NULL;
	Node* h = NULL;
	unsigned int i = 0;
	unsigned int j = cnt;

	Node* n = edge;

	while(n)
	{
		remove(n);
		insert(n, &h, &t, &i);
		n = getNearestNeighbor(n->spot);
	}

	head = h;
	tail = t;
	cnt = i;

	return (cnt==j);
}
NodeList* NodeList::getNonCommonSpotsList(NodeList* other)
{
	NodeList* res = new NodeList();

	Node* n1 = head;

	while(n1)
	{
		Node* n2 = other->head;
		bool b = false;

		while(n2 && !b)
		{
			b |= (n1->spot == n2->spot);
			n2 = n2->next;
		}

		if(!b)
		{
			res->insert(n1->spot);
		}

		n1 = n1->next;
	}

	return res;
}
Spot* NodeList::getOneNotCommonSpot(NodeList* other)
{
	Spot* spot = NULL;

	Node* n1 = head;	

	bool b = true;

	while(n1 && b)
	{
		Node* n2 = other->head;
		b = false;

		while(n2 && !b)
		{
			b |= (n1->spot == n2->spot);
			n2 = n2->next;
		}

		if(!b)
		{
			spot = n1->spot;
		}

		n1 = n1->next;
	}

	return spot;
}
NodeList* NodeList::getCommonSpotsList(NodeList* other)
{
	NodeList* res = new NodeList();

	Node* n1 = head;	

	while(n1)
	{
		Node* n2 = other->head;
		bool b = false;

		while(n2 && !b)
		{
			b |= (n1->spot == n2->spot);
			n2 = n2->next;
		}

		if(b)
		{
			res->insert(n1->spot);
		}

		n1 = n1->next;
	}

	return res;
}
Spot* NodeList::getOneCommonSpot(NodeList* other)
{
	Spot* spot = NULL;

	Node* n1 = head;	

	bool b = false;

	while(n1 && !b)
	{
		Node* n2 = other->head;
		while(n2 && !b)
		{
			b |= (n1->spot == n2->spot);
			n2 = n2->next;
		}

		if(b)
		{
			spot = n1->spot;
		}

		n1 = n1->next;
	}

	return spot;
}
Node* NodeList::find(const Spot* spot) const
{
	if(spot == NULL)
		return NULL;

	Node* node = head;

	while(node && (node->spot!=spot))
	{
		node = node->next;
	}

	return node;
}
Node* NodeList::find(const double x1, const double x2, const double y1, const double y2) const
{
	Node* node = head;

	while (node && !checkLocation(node->spot, x1, x2, y1, y2))
	{
		node = node->next;
	}

	return node;
}
Node* NodeList::getNextLineSpot(const Spot* current, const double dx,  double dy) const
{
	if(!current)
		return NULL;

	double x = current->centerGrid.x;
	double y = current->centerGrid.y;

	double d = sqrt(pow(dx,2)+pow(dy,2));
	double tolerance = d * Definitions::PositionTolerance;

	double x1 = x + dx - tolerance;
	double x2 = x + dx + tolerance;
	double y1 = y + dy - tolerance;
	double y2 = y + dy + tolerance;

	Node* node = find(x1, x2, y1, y2);

	return node;
}
bool NodeList::isExist(Spot* spot)
{
	bool b = (find(spot)!=NULL);

	return b;
}








