//
//  subject.cpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-08
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "graph/ccoms/subject.hpp"
#include "graph/ccoms/iobserver.hpp"

#ifdef subject_hpp

namespace ccoms
{

bool reactive_node::safe_destroy (void)
{
	if (suicidal)
	{
		// deletion logic
		delete this; // TODO: implement factory to ensure this is always on heap
		return true;
	}
	return false;
}
	
void subject::merge_leaves (std::unordered_set<ccoms::subject*>& src)
{
	src.emplace(this);
}

bool subject::no_audience (void)
{
	return audience_.empty();
}

subject::~subject (void)
{
	auto it = audience_.begin();
	while (audience_.end() != it)
	{
		// when an observer is destroyed, 
		// the observer attempts to detach itself from its subjects
		// that's why we increment iterator before we delete
		iobserver* captive = *it;
		it++;
		captive->safe_destroy(); // flag captive for destruction
	}
}

void subject::attach (iobserver* viewer)
{
	audience_.emplace(viewer);
}

void subject::detach (iobserver* viewer)
{
	audience_.erase(viewer);
	if (suicidal() && audience_.empty())
	{
		safe_destroy();
	}
}

void subject::notify (subject* caller)
{
	for (iobserver* viewer : audience_)
	{
		viewer->update(caller);
	}
}

}

#endif