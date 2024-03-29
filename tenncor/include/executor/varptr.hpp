//
//  varptr.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-13.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "graph/variable/placeholder.hpp"

#pragma once
#ifndef varptr_hpp
#define varptr_hpp

namespace nnet
{

// tensor variable pointer wrapper
// TODO: override delete to destroy ptr directly
// TODO: ivariable conversion unboxing would be helpful
template <typename T>
class varptr
{
	protected:
		ivariable<T>* ptr_ = nullptr;

	public:
		varptr (void) {}
		varptr (ivariable<T>* ptr);
		virtual ~varptr (void) {}
		varptr<T>& operator = (ivariable<T>* other);
		varptr<T>& operator = (const varptr<T>& other);
		
		// TODO overload move

		operator ivariable<T>* () const;
		ivariable<T>& operator * (void) const;
		ivariable<T>* operator -> (void) const;

		ivariable<T>* get (void) const;
};

template <typename T>
class placeptr : public varptr<T>
{
	public:
		placeptr (void) {}
		placeptr (placeholder<T>* ptr);
		placeptr<T>& operator = (placeholder<T>* other);
		placeptr<T>& operator = (const placeptr<T>& other);

		placeptr<T>& operator = (std::vector<T> vec);
		placeptr<T>& operator = (tensor<T>& ten);

		operator placeholder<T>* () const;
		placeholder<T>& operator * (void);
		placeholder<T>* operator -> (void);

		placeholder<T>* get (void) const;
};

}

#include "../../src/executor/varptr.ipp"

#endif /* varptr_hpp */
