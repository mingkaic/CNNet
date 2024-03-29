//
//  ram_alloc.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-11.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "iallocator.hpp"

#pragma once
#ifndef ram_alloc_hpp
#define ram_alloc_hpp

namespace nnet
{

class ram_alloc : public iallocator
{
	protected:
		virtual void* get_raw (size_t alignment,
			size_t num_bytes, const alloc_attrib& attrib) const;
		virtual void del_raw (void* ptr) const;

		virtual iallocator* clone_impl (void);

	public:
		ram_alloc* clone (void);
		virtual size_t id (void) { return 0; }
};

}

#endif /* ram_alloc_hpp */
