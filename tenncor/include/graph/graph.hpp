//
//  graph.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-12-06.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include "iconnector.hpp"

#pragma once
#ifndef graph_hpp
#define graph_hpp

namespace nnet
{
	
template <typename T>
using BUILD_GRAPH = std::function<ivariable<T>*(varptr<T>)>;

template <typename T>
class graph : public iconnector<T>
{
	private:
		std::vector<graph<T>*> succession_;
		BUILD_GRAPH<T> builder_;
		ivariable<T>* root_ = nullptr; // delay instantiated
		
		// mimics its dependency in every way, acts as an insolator to safely destroy graph
		// also be smart and actually allocate on heap (no static builder_ here)
		class buffer : public iconnector<T>
		{
			private:
				buffer (const buffer& other, std::string name) : iconnector<T>(other, name) {}
				virtual ivariable<T>* clone_impl (std::string name)
				{
					return new buffer(*this, name);
				}
				
				ivariable<T>* get (void) const { return sub_to_var<T>(this->dependencies_[0]); }
		
			public:
				buffer (ivariable<T>* var) : 
					iconnector<T>(std::vector<ivariable<T>*>{var}, var->get_name()) {}
				buffer* clone (std::string name = "") { return static_cast<buffer*>(clone_impl(name)); }
		
				virtual tensorshape get_shape (void) { return get()->get_shape(); }
				virtual tensor<T>* get_eval (void) { return get()->get_eval(); }
				virtual ivariable<T>* get_gradient (void) { return get()->get_gradient(); }
				virtual graph<T>* get_jacobian (void)
				{
					if (iconnector<T>* c = dynamic_cast<iconnector<T>*>(get()))
					{
						return c->get_jacobian();
					}
					return nullptr;
				}
		
				// directly pass notification directly to audience
				virtual void update (ccoms::caller_info info, ccoms::update_message msg = ccoms::update_message())
				{ this->notify(msg); }
		};
		
		buffer* leaf_ = nullptr;
		
		void remake_leaf (void) // refreshes entire graph
		{
			if (leaf_)
			{
				delete leaf_;
			}
			leaf_ = new buffer(sub_to_var<T>(this->dependencies_[0]));
			root_ = nullptr;
		}
	
		// COPY
		void copy (const graph& other, std::string name = "") 
		{
			succession_ = other.succession_;
			builder_ = other.builder_;
			iconnector<T>::copy(other, name);
			remake_leaf();
		}
		graph (const graph<T>& other, graph<T>* back) : 
			succession_(other.succession_),
			builder_(other.builder_),
			iconnector<T>(*back, "")
		{
			remake_leaf();
			if (back != &other)
			{
				succession_.push_back(back);
			}
		}
		virtual ivariable<T>* clone_impl (std::string name) { return new graph(*this, this); }
		
		graph (ivariable<T>* leaf,  BUILD_GRAPH<T> build) :
			builder_(build),
			iconnector<T>(std::vector<ivariable<T>*>{leaf}, "") { remake_leaf(); }

	public:
		static graph* build (ivariable<T>* leaf, 
			std::function<ivariable<T>*(varptr<T>)> build)
		{
			if (nullptr == leaf) return nullptr;
			return new graph(leaf, build);
		}
		
		virtual ~graph (void) { delete leaf_; } // kill graph

		// COPY
		graph* clone (std::string name = "")
		{
			return static_cast<graph*>(clone_impl(name));
		}
		graph& operator = (const graph& other)
		{
			if (this != &other)
			{
				copy(other);
			}
			return *this;
		}
			
		ivariable<T>* init (void)
		{
			if (nullptr == root_)
			{
				root_ = leaf_;
				for (auto rit = succession_.rbegin(); succession_.rend() != rit; rit++)
				{
					root_ = ((*rit)->builder_)(root_);
				}
				root_ = builder_(root_);
			}
			return root_;
		}
		
		// spawn a new graph appending input leaf to this
		virtual graph<T>* append_leaf (ivariable<T>* base_root) { return new graph<T>(base_root, builder_); }
		// make new graph with appending other's root to this leaf
		virtual graph<T>* append_graph (graph<T>* other) { return new graph(*this, other); }

		virtual tensorshape get_shape (void) { return init()->get_shape(); }
		virtual tensor<T>* get_eval (void) { return init()->get_eval(); }
		virtual ivariable<T>* get_gradient (void) { return init()->get_gradient(); }
		virtual graph<T>* get_jacobian (void) {
			if (iconnector<T>* c = dynamic_cast<iconnector<T>*>(init()))
			{
				return c->get_jacobian();
			}
			return nullptr;
		}
		
		virtual void update (ccoms::caller_info info, ccoms::update_message msg = ccoms::update_message())
		{ this->notify(msg); }
};
	
}

#endif /* graph_hpp */
