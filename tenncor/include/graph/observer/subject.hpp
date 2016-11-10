//
//  subject.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-08
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#pragma once
#ifndef subject_hpp
#define subject_hpp

#include <unordered_set>

namespace ccoms {

class iobserver;
class inode;

// AKA leaf
// subject retains control over all its observers,
// once destroyed, all observers are destroyed

class subject {
	private:
		std::unordered_set<iobserver*> audience_;
		// raw_data
		
	protected:
		virtual void merge_leaves (std::unordered_set<ccoms::subject*>& src) {
			src.emplace(this);
		}
		
		virtual bool no_audience (void) {
			return audience_.empty();
		}

		friend class inode;
		
	public:
		virtual ~subject (void);
	
		void attach (iobserver* viewer) {
			audience_.emplace(viewer);
		}
		
		virtual void detach (iobserver* viewer) {
			audience_.erase(viewer);
		}
		
		virtual void notify (void);
};

}

#endif /* subject_hpp */
