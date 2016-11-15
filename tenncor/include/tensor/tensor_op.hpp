//
//  tensor.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-1.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#pragma once
#ifndef tensor_op_hpp
#define tensor_op_hpp

#include <functional>
#include "tensor.hpp"

namespace nnet
{

template <typename T>
using TEN_OP = std::function<void(T*&,std::vector<const T*>)>;

// operates by pull protocol (non-reactive)
template <typename T>
class tensor_op : public tensor<T>
{
	private:
		TEN_OP<T> op_;
		std::vector<const T*> raws_;

	protected:
		void copy (const tensor_op<T>& other);
		tensor_op (const tensor_op<T>& other);
		virtual tensor<T>* clone_impl (void);

		// inherited and override. evaluates before returning raw. 
		// told you it's not overengineered
		virtual T* get_raw (void);

	public:
		tensor_op (TEN_OP<T> op);
		tensor_op (TEN_OP<T> op, iallocator& alloc);
		tensor_op (TEN_OP<T> op, iallocator* alloc);
		tensor_op (TEN_OP<T> op, iallocator& alloc, const alloc_attrib& attrib);
		tensor_op (TEN_OP<T> op, iallocator* alloc, const alloc_attrib& attrib);
		// we don't own any of the raws so we don't need destructor

		tensor_op<T>* clone (void);
		tensor_op<T>& operator = (const tensor_op<T>& other);

		// buffer arguments
		// null tensors's raw are recorded as null as well
		virtual const tensor_op<T>& operator () (std::vector<tensor<T> const*> args);
};

}

#include "../../src/tensor/tensor_op.ipp"

#endif /* tensor_op_hpp */
