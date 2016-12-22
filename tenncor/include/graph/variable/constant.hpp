//
//  constant.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-08-29.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include <list>
#include <ctime>
#include <random>
#include <new>
#include <memory>
#include "ileaf.hpp"

#pragma once
#ifndef constant_hpp
#define constant_hpp

namespace nnet
{

template <typename T>
class constant : public ileaf<T>
{
	protected:
		constant (T scalar);
		constant (std::vector<T> raw, tensorshape shape);

	public:
		// build are necessary for suicidal leaves
		static constant<T>* build (T scalar)
		{
			return new constant<T>(scalar);
		}

		static constant<T>* build (std::vector<T> raw, tensorshape shape)
		{
			return new constant<T>(raw, shape);
		}

		// COPY
		virtual constant<T>* clone (void);

		// DATA EXPOSURE TO PARENT/DEPENDENT NODES
		virtual ivariable<T>* get_gradient (void)
		{
			if (nullptr == this->grad_)
			{
				this->grad_ = std::make_unique<variable<T> >(0, "0");
			}
			return this->grad_.get();
		}
};

}

#include "../../../src/graph/variable/constant.ipp"

#endif /* constant_hpp */