//
//  dqn.hpp
//  cnnet
//
//  Created by Mingkai Chen on 2016-08-29.
//  Copyright © 2016 Mingkai Chen. All rights reserved.
//

#include <vector>
#include <cassert>

#include "gd_net.hpp"

#pragma once
#ifndef dqn_hpp
#define dqn_hpp

namespace nnet {

class dq_net {
	private:
		// argument memorization
		size_t n_observations; // input
		size_t n_actions; // output

		ml_perceptron* q_net;
		ml_perceptron* target_net;

		double rand_action_prob;
		size_t exploration_period;
		size_t store_interval;
		size_t train_interval;
		size_t mini_batch_size;
		double discount_rate;
		size_t max_exp;
		double update_rate;

		// state
		struct exp_batch {
			size_t observation;
			size_t action;
			double reward;
			size_t new_observation;
		};

		size_t actions_executed;
		std::deque<exp_batch> experiences;
		size_t iteration;
		size_t n_store_called;
		size_t n_train_called;

	public:
		dq_net (
			size_t n_input,
			std::vector<IN_PAIR> hiddens,
			size_t train_interval = 5,
			double rand_action_prob = 0.05,
			double discount_rate = 0.95,
			double update_rate = 0.01,
			// memory parameters
			size_t store_interval = 5,
			size_t mini_batch_size = 32,
			size_t max_exp = 30000);

		virtual ~dq_net (void) { delete q_net; delete target_net; }

		std::vector<double> operator () (std::vector<double>& input);

		void train (std::vector<std::vector<double> > train_batch);
};

}

#endif /* dqn_hpp */
