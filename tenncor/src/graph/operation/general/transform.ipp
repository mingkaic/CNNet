//
//  transform.ipp
//  cnnet
//
//  Created by Mingkai Chen on 2016-11-09.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#ifdef transform_hpp

namespace nnet
{

// Collect Operations

template <typename T>
void transform<T>::setup_gradient (void)
{
	std::vector<ivariable<T>*> args;
	for (ccoms::subject* child : this->dependencies_)
	{
		if (ivariable<T>* arg = child->to_type<ivariable<T> >())
		{
			args.push_back(arg);
		}
	}
	this->grad_ = dynamic_cast<ioperation<T>*>(der_(args));
}

template <typename T>
ivariable<T>* transform<T>::clone_impl (std::string name)
{
	return new transform<T>(*this, name);
}

template <typename T>
tensorshape transform<T>::shape_eval (void)
{
	tensorshape first;
	for (ccoms::subject* sub : this->dependencies_)
	{
		if (ivariable<T>* v = sub->to_type<ivariable<T> >())
		{
			first = shape_(v->get_shape());
		}
	}
	return first;
}

template <typename T>
transform<T>::transform (const transform<T>& other, std::string name) :
	collect_(other.collect_),
	shape_(other.shape_),
	der_(other.der_),
	ioperation<T>(other, name) {}

template <typename T>
transform<T>::transform (ivariable<T>* arg,
	std::function<void(T*,const T*,tensorshape)> op,
	std::function<tensorshape(tensorshape)> trans,
	BUILD_DERIVE<T> der, std::string name) :
	collect_(op), 
	shape_(trans), 
	der_(der),
	ioperation<T>(std::vector<ivariable<T>*>{arg}, name)
{
	this->out_ = std::make_unique<tensor_op<T> >(
	[this](shapeinfo info, T* dest, std::vector<const T*> srcs)
	{
		collect_(dest, srcs[0], info.res_shape_);
	},
	[trans](std::vector<tensorshape> shapes)
	{
		return trans(shapes[0]);
	});
	// try to update
	update(nullptr);
	if (session::pre_shape_eval())
	{
		shape_eval();
	}
}

template <typename T>
transform<T>* transform<T>::clone (std::string name)
{
	return static_cast<transform<T>*>(clone_impl(name));
}

template <typename T>
transform<T>& transform<T>::operator = (const transform<T>& other)
{
	if (this != &other)
	{
		collect_ = other.collect_;
		shape_ = other.shape_;
		der_ = other.der_;
		this->copy(other);
	}
	return *this;
}

template <typename T>
void transform<T>::update (ccoms::update_message msg)
{
	// cast caller dependency as ivariable
	ivariable<T>* caller = this->dependencies_[0]->template to_type<ivariable<T> >();
	assert(msg.caller_idx_ == 0); // transform is a univariable function
	
	// grad must be a leaf
	ileaf<T>* grad = nullptr;
	if (msg.grad_)
	{
		grad = msg.grad_->to_type<ileaf<T> >();
	}
	
	tensor<T>* storage;
	if (nullptr == grad) // don't care about grad, get best evaluation
	{
		storage = caller->get_eval();
	}
	else
	{
		storage = grad == caller ? grad->get_eval() : nullptr;
	}
	
	this->valid_tensor_ = nullptr != storage; // no point in operating if nullptr
	if (this->valid_tensor_)
	{
		// null is treated as erroneous zero
		(*this->out_)(std::vector<tensor<T>*>{storage});
	}

	this->notify();
}

template <typename T>
varptr<T> transpose (const varptr<T> a)
{
	if (nullptr == a) return nullptr;
	ivariable<T>* op = transform<T>::build(a,
		[](T* dest, const T* src, tensorshape ts)
		{
			// we have the new shape
			std::vector<size_t> inl = ts.as_list();
			// old dimensions
			size_t dimX = inl[1]; size_t dimY = inl[0];
			// not in place so x = y+1 doesn't work
			for (size_t y = 0; y < dimY; y++)
			{
				for (size_t x = 0; x < dimX; x++)
				{
					dest[y+x*dimY] = src[x+y*dimX];
				}
			}
		},
		[](tensorshape ts)
		{
			if (ts.is_fully_defined())
			{
				// restrict shape to no greater than 2-D for now
				assert(ts.n_dims() <= 2);
				std::vector<size_t> inl = ts.as_list();
				if (ts.n_dims() == 1)
				{
					return std::vector<size_t>{1, inl[0]};
				}
				return std::vector<size_t>{inl[1], inl[0]};
			}
			return std::vector<size_t>{};
		},
		[](std::vector<ivariable<T>*> args)
		{
			ivariable<T>* a = args.front();
			return transpose(varptr<T>(a->get_gradient()));
		},
	"transpose(" + a->get_name() + ")");
	return op;
}

// fit to watch
template <typename T>
varptr<T> fit (const varptr<T> a, const varptr<T> watch)
{
	if (nullptr == a && nullptr == watch) return nullptr;
	// additional constraint that watch shape must be have shape with
	// dimensions greater or equal to a's dimensional value (shape.as_list()[i])
	ivariable<T>* op = transform<T>::build(a,
		[watch, a](T* dest, const T* src, tensorshape ts)
		{
			std::vector<size_t> orig = a->get_shape().as_list();
			std::vector<size_t> tv = ts.as_list();
			size_t total = ts.n_elems();

			T temp[ts.n_elems()];
			T temp2[ts.n_elems()];

			const T* super_src = src;
			T* super_dest = temp;
			size_t below_dim = 1;

			for (size_t index = 0; index < tv.size(); index++)
			{
				below_dim *= tv[index];
				size_t mult = 0;
				if (index < orig.size())
				{
					assert(orig[index] <= tv[index]);
					if (0 == orig[index] % tv[index])
					{
						mult = tv[index] / orig[index];
					}
				}
				else
				{
					mult = tv[index];
				}
				if (mult)
				{
					size_t below = below_dim * tv[index] / mult;
					size_t above = total / below;

					// copy over data
					const T* src_addr = super_src;
					for (size_t i = 0; i < above; i++)
					{
						// copy data mult times
						src_addr += i * below;
						for (size_t j = 0; j < mult; j++)
						{
							T* dest_addr = super_dest + below * (mult * i + j);
							std::memcpy(dest_addr, src_addr, below * sizeof(T));
						}
					}
					// state update: below_dim, super_src, and super_dest
					below_dim *= mult;
					if (super_src == temp)
					{
						super_src = temp2;
						super_dest = temp;
					}
					else
					{
						super_src = temp;
						super_dest = temp2;
					}
				}
			}
			std::memcpy(dest, super_dest, total * sizeof(T));
		},
		[watch](tensorshape ts)
		{
			ts.assert_is_fully_defined();
			tensorshape s = watch->get_shape();
			assert(s.n_elems() >= ts.n_elems());
			return s;
		},
		[watch](std::vector<ivariable<T>*> args)
		{
			ivariable<T>* a = args.front();
			return fit(varptr<T>(a->get_gradient()), watch);
		},
	nnutils::formatter() << "fit[" << watch->get_name() <<  "](" << a->get_name() + ")");
	return op;
}

template <typename T>
varptr<T> extend (const varptr<T> a, size_t index, size_t multiplier)
{
	if (nullptr == a && 1 >= multiplier) return nullptr;
	ivariable<T>* op = transform<T>::build(a,
		[index, multiplier](T* dest, const T* src, tensorshape ts)
		{
			// REMEMBER that ts is the resulting shape, not the original shape
			// both above and below values are calculations based on the original shape
			std::vector<size_t> tv = ts.as_list();
			// below calculates all elements encompassed up to the index dimension
			// that is for a shape of <1, 2, 3, 4> and index 2
			// below = 1 * 2 * 3 = 6
			size_t below = 1;
			for (size_t i = 0; i < index; i++)
			{
				below *= tv[i];
			}
			// we know that for the resulting shape, the dimensional-value at index is multiplied by multiplier
			// so to obtain the original dimension, we divide by multiplier
			below *= tv[index] / multiplier;
			// above calculates the number of tensors (of index rank) within the original tensor
			// that is for a shape of <1, 2, 3, 4> and index 2
			// the tensors of index rank is represented by the first 3 dimensions <1, 2, 3>
			// the overall tensor is represented as a tensor of tensor < <1, 2, 3>, 4>
			// above is 4
			// above = original total / below
			// original total = resulting total / multiplier
			size_t above = ts.n_elems() / (multiplier * below);

			// copy over data
			for (size_t i = 0; i < above; i++)
			{
				// copy data multiplier times
				const T* src_addr = src + i * below;
				for (size_t j = 0; j < multiplier; j++)
				{
					T* dest_addr = dest + below * (multiplier * i + j);
					std::memcpy(dest_addr, src_addr, below * sizeof(T));
				}
			}
		},
		[index, multiplier](tensorshape ts)
		{
			ts.assert_is_fully_defined();
			std::vector<size_t> tv = ts.as_list();
			// allocated additional space along index
			size_t dims = ts.n_dims();
			if (index >= dims)
			{
				// extending extra dimensions
				size_t extra_dims = index - dims;
				if (extra_dims)
				{
					tv.insert(tv.end(), extra_dims, 1);
				}
				tv.push_back(multiplier);
			}
			else
			{
				tv[index] *= multiplier;
			}
			return tv;
		},
		[index, multiplier](std::vector<ivariable<T>*> args)
		{
			ivariable<T>* a = args.front();
			return extend(varptr<T>(a->get_gradient()), index, multiplier);
		},
	nnutils::formatter() << "extend[" << index << "," <<
		multiplier << "](" << a->get_name() + ")");
	return op;
}

template <typename T>
varptr<T> compress (const varptr<T> a, int index,
	std::function<T(const std::vector<T>&)> collector)
{
	if (nullptr == a) return nullptr;
	std::function<void(T*,const T*,tensorshape)> gatherer;
	std::function<tensorshape(tensorshape)> shaper;
	if (index >= 0)
	{
		gatherer = std::function<void(T*,const T*,tensorshape)>(
		[index, collector, a](T* dest, const T* src, tensorshape ts)
		{
			// REMEMBER that ts is the resulting shape, not the original shape
			// both above and below values are calculations based on the original shape
			// original shape
			tensorshape orig = a->get_shape();
			assert((unsigned) index < orig.n_dims());
			std::vector<size_t> tv = orig.as_list();
			size_t idx_val = tv[index];
			// below for compression calculates all elements below the index dimension
			// that is for a shape of <1, 2, 3, 4> and index 2
			// below = 1 * 2 = 2
			size_t below = 1;
			for (int i = 0; i < index; i++)
			{
				below *= tv[i];
			}
			// above denotes the same above as the one in extend
			size_t above = orig.n_elems() / (below*idx_val);

			// copy over data
			for (size_t i = 0; i < above; i++)
			{
				for (size_t j = 0; j < below; j++)
				{
					// apply compression to each element along idx_val dimension
					size_t dest_idx = j + i * below;
					std::vector<T> gather;
					for (size_t k = 0; k < idx_val; k++)
					{
						gather.push_back(src[j + k * below + i * below * idx_val]);
					}
					dest[dest_idx] = collector(gather);
				}
			}
		});
		shaper = std::function<tensorshape(tensorshape)>(
		[index](tensorshape ts)
		{
			ts.assert_is_fully_defined();
			assert((unsigned) index < ts.n_dims());
			std::vector<size_t> tv = ts.as_list();
			if (0 == index)
			{ // pop front
				tv.front() = std::move(tv.back());
				tv.pop_back();
			}
			else if (tv.size()-1 == (unsigned) index)
			{
				tv.pop_back();
			}
			else
			{
				tv[index] = 1;
			}
			return tv;
		});
	}
	else
	{
		gatherer = std::function<void(T*,const T*,tensorshape)>(
		[collector, a](T* dest, const T* src, tensorshape ts)
		{
			std::vector<size_t> tv = ts.as_list();
			size_t total = ts.n_elems();
			dest[0] = collector(std::vector<T>(dest, dest+total));
		});
		shaper = std::function<tensorshape(tensorshape)>(
		[index](tensorshape ts)
		{
			return std::vector<size_t>{1};
		});
	}

	ivariable<T>* op = transform<T>::build(a, gatherer, shaper,
		[index, collector](std::vector<ivariable<T>*> args)
		{
			ivariable<T>* a = args.front();
			return compress(varptr<T>(a->get_gradient()), index, collector);
		},
	nnutils::formatter() << "compress[" << index << "](" << a->get_name() + ")");
	return op;
}

}

#endif