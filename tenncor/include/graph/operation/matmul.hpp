//
//  mat_ops.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-10-08
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "graph/operation/elementary.hpp"
#include "graph/operation/transform.hpp"
#include "graph/variable/constant.hpp"
#include "graph/graph.hpp"

#pragma once
#ifndef matop_hpp
#define matop_hpp

namespace nnet
{

// MATRIX MULTIPLICATION

// restricted to 2-d matrices with proper shapes
// dimension 1 denote number of columns,
// dimension 2 denote number of rows
template <typename T>
class matmul : public ioperation<T>
{
	private:
		bool transposeA_;
		bool transposeB_;

		size_t common_dim (void);

	protected:
		// backward chaining for AD
		virtual void setup_gradient (void);
		
		matmul (const matmul<T>& other, std::string name);
		virtual ivariable<T>* clone_impl (std::string name);

		// protect matrix constructor to ensure heap allocation
		matmul (ivariable<T>* a, ivariable<T>* b,
			bool transposeA = false, bool transposeB = false);

	public:
		static matmul<T>* build (ivariable<T>* a, ivariable<T>* b,
			bool transposeA = false, bool transposeB = false)
		{
			return new matmul<T>(a, b, transposeA, transposeB);
		}

		// COPY
		matmul<T>* clone (std::string name = "");
		virtual matmul<T>& operator = (const matmul<T>& other);
		
		// MOVES
		// TODO: implement

		// override ioperation
		virtual void update (ccoms::caller_info info, ccoms::update_message msg = ccoms::update_message());
};

}

#include "../../../src/graph/operation/matmul.ipp"

#endif /* matop_hpp */
