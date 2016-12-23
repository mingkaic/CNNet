//
//  ileaf.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-08-29.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "graph/ivariable.hpp"

#pragma once
#ifndef ileaf_hpp
#define ileaf_hpp

namespace nnet
{

// INITIALIZER MANAGING INTERFACE
// Leaf Nodes

template <typename T>
class ileaf : public ivariable<T>
{
	private:
		void copy (const ileaf<T>& other);
	
	protected:
		// >>>> TENSOR CONTENT <<<<
		std::unique_ptr<tensor<T> > out_ = nullptr;

		// >>>> INITIALIZER DATA <<<<
		initializer<T>* init_ = nullptr;
		bool is_init_ = false;

		// used by assignment operators to dynamically initialize tensors
		struct dyn_init;
		
		// only add to source if this is a ivariable
		virtual void merge_leaves (std::unordered_set<ivariable<T>*>& src) {}

		ileaf (const ileaf<T>& other); // copy constructor required for out_, grad_, and init_ deep copy

		ileaf (const tensorshape& shape, initializer<T>* init, std::string name);

		friend class assign<T>;

	public:
		virtual ~ileaf (void);
		
		// COPY
		// abstract clone
		ileaf<T>& operator = (const ileaf<T>& other);

		// MOVES
		// todo: implement move clone

		virtual tensorshape get_shape (void)
		{
			if (nullptr != out_)
			{
				return out_->get_shape();
			}
			return std::vector<size_t>{};
		}

		// inherited from ivariable
		virtual tensor<T>* get_eval (void)
		{
			if (false == is_init())
			{
				return nullptr;
			}
			return out_.get();
		}

		// DATA EXPOSURE TO PARENT/DEPENDENT NODES
		virtual bindable_toggle<T>* get_gradient (void) { return nullptr; }

		// GET INFO
		bool can_init (void) const;
		bool is_init (void) const { return is_init_; }
};

}

#include "../../../src/graph/variable/ileaf.ipp"

#endif /* ileaf_hpp */
