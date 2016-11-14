//
//  allocator.cpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-08-29.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "memory/iallocator.hpp"

#ifdef allocator_hpp

namespace nnet {

void* iallocator::get_raw (size_t alignment,
	size_t num_bytes) {
	return get_raw(alignment, num_bytes, alloc_attrib());
}

size_t iallocator::requested_size (void* ptr) {
	throw std::bad_function_call();
	return 0;
}

}

#endif