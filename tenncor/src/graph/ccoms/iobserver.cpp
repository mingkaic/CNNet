//
//  iobserver.cpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-08
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#ifdef observer_hpp

#include "graph/ccoms/iobserver.hpp"

namespace ccoms
{
		
void iobserver::add_dependency (ccoms::subject* dep)
{
	dependencies_.push_back(dep);
	dep->attach(this);
	dep->merge_leaves(leaves_);
}

void iobserver::merge_leaves (std::unordered_set<ccoms::subject*>& src)
{
	src.insert(this->leaves_.begin(), this->leaves_.end());
}

iobserver::iobserver (std::vector<ccoms::subject*> dependencies)
{
	for (ccoms::subject* dep : dependencies)
	{
		add_dependency(dep);
	}
}
		
iobserver::~iobserver (void)
{
	for (ccoms::subject* dep : dependencies_)
	{
		dep->detach(this);
	}
}
		
void iobserver::leaves_collect (std::function<void(subject*)> collector)
{
	for (ccoms::subject* leaf : leaves_)
	{
		collector(leaf);
	}
}

}

#endif