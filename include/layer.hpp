//
//  layer.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-08-29.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#pragma once
#ifndef layer_hpp
#define layer_hpp

#include <string>
#include <vector>
#include <random>
#include <functional>
#include <list>
#include <stack>

#include "tenncor/tenncor.hpp"
#include "graph/bridge/varptr.hpp"

namespace nnet {

#define WB_PAIR std::pair<ivariable<double>*, ivariable<double>*>
#define IN_PAIR std::pair<size_t, nnet::VAR_FUNC<double> >
#define HID_PAIR std::pair<layer_perceptron*, nnet::VAR_FUNC<double> >

// CONSTRAINTS: without tensors, all features are fed by vectors
// higher dimensional features must be contracted to vector or reduced in some manner
// TODO: make convolution neural net via multiple weights per layer
// weights are <output, input>
class layer_perceptron {
	private:
		static random_uniform<double> rinit;
		static const_init<double> zinit;
		static const_init<double> oinit;

		std::string scope;
		size_t n_input;
		size_t n_output;
		// any allowed size
		ivariable<double>* weights_ = nullptr;
		ivariable<double>* bias_ = nullptr;

	protected:
		void copy (const layer_perceptron& other, std::string scope);

	public:
		layer_perceptron (
			size_t n_input,
			size_t n_output,
			std::string scope="");
		virtual ~layer_perceptron (void) {}
		layer_perceptron (const layer_perceptron& other, std::string scope="");
		layer_perceptron& operator = (const layer_perceptron& other);

		// input are expected to have shape n_input by batch_size
		// outputs are expected to have shape output by batch_size
		nnet::ivariable<double>* operator () (ivariable<double>*);

		size_t get_n_input (void) const { return n_input; }
		size_t get_n_output (void) const { return n_output; }
		WB_PAIR get_variables (void) const { return WB_PAIR(weights_, bias_); }
};

class ml_perceptron {
	//private:
	protected:
		std::string scope;
		std::vector<HID_PAIR> layers;

		void copy (const ml_perceptron& other, std::string scope);

		ml_perceptron (const ml_perceptron& other, std::string scope);

	public:
		// trust that passed in operations are unconnected
		ml_perceptron (size_t n_input,
			std::vector<IN_PAIR> hiddens,
			std::string scope = "MLP");
		virtual ~ml_perceptron (void);
		ml_perceptron* clone (std::string scope = "MLP_COPY") { return new ml_perceptron(*this, scope); }
		ml_perceptron& operator = (const ml_perceptron& other);

		// input are expected to have shape n_input by batch_size
		// outputs are expected to have shape output by batch_size
		nnet::ivariable<double>* operator () (placeholder<double>* input);
		std::vector<WB_PAIR> get_variables (void);
};

}

#endif /* layer_hpp */
