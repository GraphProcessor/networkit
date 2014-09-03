/*
 * GlobalClusteringCoefficient.h
 *
 *  Created on: 12.11.2013
 */

#ifndef GLOBALCLUSTERINGCOEFFICIENT_H_
#define GLOBALCLUSTERINGCOEFFICIENT_H_

#include "../graph/Graph.h"

namespace NetworKit {

/**
 * @ingroup properties
 */
class GlobalClusteringCoefficient {

public:

	/** Default destructor */
	virtual ~GlobalClusteringCoefficient() = default;
  
	virtual double approximate(const Graph& G, int k);
};

} /* namespace NetworKit */
#endif /* GLOBALCLUSTERINGCOEFFICIENT_H_ */
