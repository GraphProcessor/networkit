/*
 * MaxentStress.h
 *
 *  Created on: 22.01.2014
 *      Author: Henning
 */

#ifndef MAXENTSTRESS_H_
#define MAXENTSTRESS_H_

#include "Layouter.h"
#include "../distmeasures/AlgebraicDistance.h"

namespace NetworKit {

/**
 * @ingroup viz
 */
class MaxentStress: public NetworKit::Layouter {
protected:


public:
	/**
	 * DO NOT USE to create objects.
	 * Default constructor. Only necessary for Python shell.
	 */
	MaxentStress() {}  // nullary constructor needed for Python shell
	MaxentStress(Point<float> bottomLeft, Point<float> topRight, bool useGivenLayout = false);
	virtual ~MaxentStress();

	virtual void draw(Graph& g);
};

} /* namespace NetworKit */
#endif /* MAXENTSTRESS_H_ */