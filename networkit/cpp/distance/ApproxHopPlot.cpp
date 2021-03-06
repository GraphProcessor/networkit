/*
* ApproxHopPlot.cpp
*
*  Created on: 16.06.2014
*      Author: Marc Nemes
*/

#include "ApproxHopPlot.h"
#include "../components/ConnectedComponents.h"
#include "../auxiliary/Random.h"

#include <math.h>
#include <iterator>
#include <stdlib.h>
#include <omp.h>
#include <map>

namespace NetworKit {

ApproxHopPlot::ApproxHopPlot(const Graph& G, const count maxDistance, const count k, const count r): Algorithm(), G(G), maxDistance(maxDistance), k(k), r(r) {
	if (G.isDirected()) throw std::runtime_error("current implementation can only deal with undirected graphs");
	ConnectedComponents cc(G);
	cc.run();
	if (cc.getPartition().numberOfSubsets() > 1) throw std::runtime_error("current implementation only runs on graphs with 1 connected component");
}

void ApproxHopPlot::run() {
	count z = G.upperNodeIdBound();
	// the length of the bitmask where the number of connected nodes is saved
	count lengthOfBitmask = (count) ceil(log2(G.numberOfNodes()));
	// saves all k bitmasks for every node of the current iteration
	std::vector<std::vector<unsigned int> > mCurr(z);
	// saves all k bitmasks for every node of the previous iteration
	std::vector<std::vector<unsigned int> > mPrev(z);
	// the maximum possible bitmask based on the random initialization of all k bitmasks
	std::vector<count> highestCount;
	// the current distance of the neighborhoods
	count h = 1;
	// the estimated number of connected nodes
	double estimatedConnectedNodes;
	// the sum over all estimated connected nodes
	double totalConnectedNodes;
	// used for setting a random bit in the bitmasks
	double random;
	// the position of the bit that has been set in a bitmask
	count position;
	// nodes that are not connected to enough nodes yet
	std::vector<node> activeNodes;

	// initialize all vectors
	highestCount.assign(k, 0);
	G.forNodes([&](node v) {
		std::vector<unsigned int> bitmasks;
		bitmasks.assign(k, 0);
		mCurr[v] = bitmasks;
		mPrev[v] = bitmasks;
		activeNodes.push_back(v);

		// set one bit in each bitmask with probability P(bit i=1) = 0.5^(i+1), i=0,..
		for (count j = 0; j < k; j++) {
			random = Aux::Random::real(0,1);
			position = ceil(log(random)/log(0.5) - 1);
			// set the bit in the bitmask
			if (position < lengthOfBitmask+r) {
				mPrev[v][j] |= 1 << position;
			}
			// add the current bit to the maximum-bitmask
			highestCount[j] = highestCount[j] | mPrev[v][j];
		}
	});
	// at zero distance, all nodes can only reach themselves
	hopPlot[0] = 1/G.numberOfNodes();
	// as long as we need to connect more nodes
	while (!activeNodes.empty() && (maxDistance <= 0 || h < maxDistance)) {
		totalConnectedNodes = 0;
		for (count x = 0; x < activeNodes.size(); x++) {
			node v = activeNodes[x];
			// for each parallel approximation
			for (count j = 0; j < k; j++) {
				// the node is still connected to all previous neighbors
				mCurr[v][j] = mPrev[v][j];
				// and to all previous neighbors of all its neighbors
				G.forNeighborsOf(v, [&](node u) {
					mCurr[v][j] = mCurr[v][j] | mPrev[u][j];
				});
			}
			// the least bit number in the bitmask of the current node/distance that has not been set
			double b = 0;

			for (count j = 0; j < k; j++) {
				for (count i = 0; i < sizeof(i)*8; i++) {
					if (((mCurr[v][j] >> i) & 1) == 0) {
						b += i;
						break;
					}
				}
			}
			// calculate the average least bit number that has not been set over all parallel approximations
			b = b / k;

			// calculate the estimated number of neighbors
			// For the origin of the factor 0.77351 see http://www.mathcs.emory.edu/~cheung/papers/StreamDB/Probab/1985-Flajolet-Probabilistic-counting.pdf Theorem 3.A (p. 193)
			estimatedConnectedNodes = (pow(2,b) / 0.77351);

			// enforce monotonicity
			if (estimatedConnectedNodes > G.numberOfNodes()) {
				estimatedConnectedNodes = G.numberOfNodes();
			}

			// check whether all k bitmask for this node have reached the highest possible value
			bool nodeFinished = true;
			for (count j = 0; j < k; j++) {
				if (mCurr[v][j] != highestCount[j]) {
					nodeFinished = false;
					break;
				}
			}

			// if the node wont change or is connected to enough nodes it must no longer be considered
			if (estimatedConnectedNodes >= G.numberOfNodes() || nodeFinished) {
				// remove the current node from future iterations
				std::swap(activeNodes[x], activeNodes.back());
				activeNodes.pop_back();
				totalConnectedNodes += G.numberOfNodes();
				--x; //don't skip former activeNodes.back() that has been switched to activeNodes[x]
			} else {
				// add value of the node to all nodes so we can calculate the average
				totalConnectedNodes += estimatedConnectedNodes;
			}
		}
		// add nodes that are already connected to all nodes
		totalConnectedNodes += (G.numberOfNodes() - activeNodes.size()) * G.numberOfNodes();
		// compute the fraction of connected nodes
		hopPlot[h] = totalConnectedNodes/(G.numberOfNodes()*G.numberOfNodes());
		if (hopPlot[h] > 1) {
			hopPlot[h] = 1;
		}
		mPrev = mCurr;
		h++;
	}
	hasRun = true;
}

std::map<count, double> ApproxHopPlot::getHopPlot() const {
	if(!hasRun) {
		throw std::runtime_error("Call run()-function first.");
	}
	return hopPlot;
}

}
