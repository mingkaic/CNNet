//
//  elementary.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-10-24.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#ifndef elementary_hpp
#define elementary_hpp

#include "graph/operation/ioperation.hpp"

namespace nnet {

template <typename T>
using ELEMENTARY_DERIV = std::function<VAR_PTR<T>(std::vector<VAR_PTR<T> >)>;

template <typename T>
class elementary : public ioperation<T> {
	protected:
		std::vector<VAR_PTR<T> > args_; // order matters
		std::function<void(T&, T)> op_;
		ELEMENTARY_DERIV<T> der_;

		virtual void make_gradient (VAR_PTR<T>& safety_ref);
		virtual EVOKER_PTR<T> clone_impl (std::string name);
		virtual void replace (ivariable<T>* food, VAR_PTR<T> newfood);
		virtual void shape_eval (void);
		elementary (std::vector<VAR_PTR<T> > args,
					std::function<void(T&, T)> op,
					ELEMENTARY_DERIV<T> der,
					std::string name = "");

	public:
		static VAR_PTR<T> make (std::vector<VAR_PTR<T> > args,
								std::function<void(T&, T)> op,
								ELEMENTARY_DERIV<T> der,
								std::string name = "") {
			return ivariable<T>::make_shared(new elementary(args, op, der, name));
		}

		std::shared_ptr<elementary<T> > clone (std::string name = "") {
			return std::static_pointer_cast<elementary<T>, ievoker<T> >(clone_impl(name));
		}

		virtual const tensor<T>& eval (void);
		
		virtual VAR_PTR<T> push_to (VAR_PTR<T> in_grad, VAR_PTR<T> end_node) {
			VAR_PTR<T> buffer;
			std::vector<VAR_PTR<T> > record;
			for (VAR_PTR<T> ar : args_) {
				// perform a look ahead to see if ar leads to the desired leaf node end_node
				if (ar->leaves_.end() != ar->leaves_.find()) {
					buffer = ar->push_to(in_grad, end_node);
					if (nullptr != buffer) {
						record.push_back(buffer);
					}
				}
			}
			if (true == record.empty()) {
				return nullptr;
			} else if (1 == record.size()) {
				return record[0];
			}
			return elementary<T>::make(record, op_, der_);
		}
};

// operators that will replace elementary operation objects
template<typename T>
VAR_PTR<T> operator + (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> operator - (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> sin (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> cos (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> tan (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> csc (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> sec (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> cot (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> exp (const VAR_PTR<T>& a);

template<typename T>
VAR_PTR<T> operator + (T a, const VAR_PTR<T>& b);

template<typename T>
VAR_PTR<T> operator + (const VAR_PTR<T>& a, T b);

template<typename T>
VAR_PTR<T> operator + (const VAR_PTR<T>& a, const VAR_PTR<T>& b);

template<typename T>
VAR_PTR<T> operator - (T a, const VAR_PTR<T>& b);

template<typename T>
VAR_PTR<T> operator - (const VAR_PTR<T>& a, T b);

template<typename T>
VAR_PTR<T> operator - (const VAR_PTR<T> &a, const VAR_PTR<T> &b);

template<typename T>
VAR_PTR<T> operator * (T a, const VAR_PTR<T>& b);

template<typename T>
VAR_PTR<T> operator * (const VAR_PTR<T>& a, T b);

template<typename T>
VAR_PTR<T> operator * (const VAR_PTR<T> &a, const VAR_PTR<T> &b);

template<typename T>
VAR_PTR<T> operator / (T a, const VAR_PTR<T>& b);

template<typename T>
VAR_PTR<T> operator / (const VAR_PTR<T>& a, T b);

template<typename T>
VAR_PTR<T> operator / (const VAR_PTR<T> &a, const VAR_PTR<T> &b);

}

#include "../../../src/graph/operation/elementary.ipp"

#endif /* elementary_hpp */
