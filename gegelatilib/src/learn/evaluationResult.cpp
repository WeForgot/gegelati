/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */
#include <stdexcept>

#include "learn/evaluationResult.h"

double Learn::EvaluationResult::getResult() const {
	return this->result;
}

size_t Learn::EvaluationResult::getNbEvaluation() const
{
	return this->nbEvaluation;
}

Learn::EvaluationResult& Learn::EvaluationResult::operator+=(const Learn::EvaluationResult& other)
{
	// Type Check (Must be done in all override)
	// This test will succeed in child class.
	const std::type_info& thisType = typeid(*this);
	if (typeid(other) != thisType) {
		throw std::runtime_error("Type mismatch between EvaluationResults.");
	}

	// If the added type is Learn::EvaluationResult
	if (thisType == typeid(Learn::EvaluationResult)) {
		// Weighted addition of results
		this->result = this->result * (double)this->nbEvaluation + other.result * (double)other.nbEvaluation;
		this->result /= (double)this->nbEvaluation + (double)other.nbEvaluation;

		// Addition ot nbEvaluation
		this->nbEvaluation += other.nbEvaluation;
	}

	return *this;
}

bool Learn::operator<(const EvaluationResult& a, const EvaluationResult& b)
{
	return a.getResult() < b.getResult();
}
