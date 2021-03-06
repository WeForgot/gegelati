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

#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "instructions/lambdaInstruction.h"
#include "data/untypedSharedPtr.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "program/programExecutionEngine.h"

class ProgramExecutionEngineTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	const double value0{ 2.3 };
	const float value1{ 0.2f };
	const double value2{ 0.5 };
	const double value3{ 1.5 };
	std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	virtual void SetUp() {
		vect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
		vect.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

		((Data::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(double), 25, value0);
		((Data::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(double), 5, value2);
		((Data::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(double), 6, value3);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));
		set.add(*new Instructions::LambdaInstruction<const double[2], const double[2]>([](const double a[2], const double b[2]) {
			return a[0] * b[0] + a[1] * b[1];
			}));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);

		Program::Line& l0 = p->addNewLine();
		l0.setInstructionIndex(0); // Instruction is addPrimitiveType<double>.
		l0.setOperand(0, 0, 5); // 1st operand: 6th register.
		l0.setOperand(1, 2, 25); // 2nd operand: 26th double in the PrimitiveTypeArray of double.
		l0.setDestinationIndex(1); // Destination is resgister at index 1

		// Intron line
		Program::Line& l1 = p->addNewLine();
		l1.setInstructionIndex(1); // Instruction is MultByConstParam<double, float>.
		l1.setOperand(0, 0, 3); // 1st operand: 3rd register.
		l1.setParameter(0, (float)value0); // Parameter is set to value1 (=2.3f)
		l1.setDestinationIndex(0); // Destination is register at index 0

		Program::Line& l2 = p->addNewLine();
		l2.setInstructionIndex(1); // Instruction is MultByConstParam<double, float>.
		l2.setOperand(0, 0, 1); // 1st operand: 1th register.
		l2.setParameter(0, value1); // Parameter is set to value1 (=0.2f)
		l2.setDestinationIndex(0); // Destination is register at index 0

		Program::Line& l3 = p->addNewLine();
		l3.setInstructionIndex(2); // Instruction is LambdaInstruction<double[2]>.
		l3.setOperand(0, 0, 0); // 1st operand: 0th and 1st registers.
		l3.setOperand(1, 2, 5);	// 2nd operand : 6th and 7th double in the PrimitiveTypeArray of double.
		l3.setDestinationIndex(0); // Destination is register at index 0

		// Mark intron lines
		ASSERT_EQ(p->identifyIntrons(), 1);
	}

	virtual void TearDown() {
		delete p;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
		delete (&set.getInstruction(2));
	}
};

TEST_F(ProgramExecutionEngineTest, ConstructorDestructor) {
	Program::ProgramExecutionEngine* progExecEng;
	ASSERT_NO_THROW(progExecEng = new Program::ProgramExecutionEngine(*p)) << "Construction failed.";

	ASSERT_NO_THROW(delete progExecEng) << "Destruction failed.";

	std::vector<std::reference_wrapper<Data::DataHandler>> vect2;
	vect2.push_back(*vect.at(0).get().clone());
	ASSERT_THROW(progExecEng = new Program::ProgramExecutionEngine(*p, vect2), std::runtime_error) << "Construction should faile with data sources differing in number from those of the Environment.";
	vect2.push_back(*vect.at(1).get().clone());

	ASSERT_NO_THROW(progExecEng = new Program::ProgramExecutionEngine(*p)) << "Construction failed with a perfect copy of the environment data source.";
	ASSERT_NO_THROW(delete progExecEng) << "Destruction failed.";

	// Push a new dataHandler instead.
	// Because its id is different, it will not be accepted by the PEE.
	delete (&(vect2.at(1).get()));
	vect2.pop_back();
	vect2.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));
	ASSERT_THROW(progExecEng = new Program::ProgramExecutionEngine(*p, vect2), std::runtime_error) << "Construction should fail with data sources differing in id from those of the Environment.";

	delete (&(vect2.at(0).get()));
	delete (&(vect2.at(1).get()));
}

TEST_F(ProgramExecutionEngineTest, next) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// 4 lines minus one intron line
	ASSERT_TRUE(progExecEng.next()) << "Program has three line so going to the next line after initialization should succeed.";
	ASSERT_TRUE(progExecEng.next()) << "Program has three line so going to the next line after initialization should succeed.";
	ASSERT_FALSE(progExecEng.next()) << "Program has three line so going to the next line three times after initialization should not succeed.";
}

TEST_F(ProgramExecutionEngineTest, getCurrentLine) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Valid since the program has more than 0 line and program counter is initialized to 0.
	ASSERT_EQ(&progExecEng.getCurrentLine(), &p->getLine(0)) << "First line of the Program not accessible from the ProgramExecutionEngine.";
	progExecEng.next(); // Skips the intron automatically
	ASSERT_EQ(&progExecEng.getCurrentLine(), &p->getLine(2)) << "Second line of the Program not accessible from the ProgramExecutionEngine.";
}

TEST_F(ProgramExecutionEngineTest, getCurrentInstruction) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Valid since the program has more than 0 line and program counter is initialized to 0.
	const Instructions::Instruction& instr = progExecEng.getCurrentInstruction();
	// First Instruction of the set (from Fixture) is Instructions::AddPrimitiveType<double>.
	// Since the Line was initialized to 0, its instruction index is 0.
	ASSERT_EQ(typeid(instr), typeid(Instructions::AddPrimitiveType<double>)) << "The type of the instruction does not correspond to the Set of the Environment.";
}

TEST_F(ProgramExecutionEngineTest, fetchOperands) {
	Program::ProgramExecutionEngine progExecEng(*p);
	std::vector<Data::UntypedSharedPtr> operands;

	progExecEng.next();
	progExecEng.next();

	// From Fixture:
	// Program line 3
	// Instruction is LambdaInstruction<double[]>
	// Operands are: index 0 and 1 registers register and index 5 and 6 elements of an double array.
	ASSERT_NO_THROW(progExecEng.fetchCurrentOperands(operands)) << "Fetching the operands of a valid Program from fixtures failed.";
	// Check number of operands
	ASSERT_EQ(operands.size(), 2) << "Incorrect number of operands were fetched by previous call.";
	// Check operand value. Registers are 0.0, array element is value2 and value3
	ASSERT_EQ(((operands.at(0)).getSharedPointer<const double[]>())[0], 0.0) << "Value of fetched operand from register is incorrect.";
	ASSERT_EQ(((operands.at(0)).getSharedPointer<const double[]>())[1], 0.0) << "Value of fetched operand from register is incorrect.";
	ASSERT_EQ(((operands.at(1)).getSharedPointer<const double[]>())[0], value2) << "Value of fetched operand from array is incorrect.";
	ASSERT_EQ(((operands.at(1)).getSharedPointer<const double[]>())[1], value3) << "Value of fetched operand from array is incorrect.";
}

TEST_F(ProgramExecutionEngineTest, fetchCompositeOperands) {
	Program::ProgramExecutionEngine progExecEng(*p);
	std::vector<Data::UntypedSharedPtr> operands;
	// From Fixture:
	// Program line 0
	// Instruction is AddPrimitiveType<double>.
	// Operands are: 6th (index = 5) register and 26th (index = 25) element of an double array.
	ASSERT_NO_THROW(progExecEng.fetchCurrentOperands(operands)) << "Fetching the operands of a valid Program from fixtures failed.";
	// Check number of operands
	ASSERT_EQ(operands.size(), 2) << "Incorrect number of operands were fetched by previous call.";
	// Check operand value. Register is 0.0, array element is value0: 2.3
	ASSERT_EQ((double)*((operands.at(0)).getSharedPointer<const double>()), 0.0) << "Value of fetched operand from register is incorrect.";
	ASSERT_EQ((double)*((operands.at(1)).getSharedPointer<const double>()), value0) << "Value of fetched operand from array is incorrect compared to Test fixture.";
}

TEST_F(ProgramExecutionEngineTest, fetchParameters) {
	Program::ProgramExecutionEngine progExecEng(*p);
	std::vector<std::reference_wrapper<const Parameter>> parameters;

	// First line of fixture has no parameters. Just check that nothing is thrown.
	ASSERT_NO_THROW(progExecEng.fetchCurrentParameters(parameters)) << "Fetching the parameters of a valid Program from fixtures failed.";
	ASSERT_EQ(parameters.size(), 0) << "Since first line of the Program refers to an instruction using no Parameter, the vector should remain empty.";
	progExecEng.next();

	ASSERT_NO_THROW(progExecEng.fetchCurrentParameters(parameters)) << "Fetching the parameters of a valid Program from fixtures failed.";
	// Check number of parameters
	ASSERT_EQ(parameters.size(), 1) << "Incorrect number of operands were fetched by previous call.";
	// Check parameter value (set in fixture). value1: 0.2f (+/- the parameter floating precision)
	ASSERT_NEAR((float)parameters.at(0).get(), value1, PARAM_FLOAT_PRECISION) << "Value of fetched parameter is incorrect.";
}

TEST_F(ProgramExecutionEngineTest, executeCurrentLine) {
	Program::ProgramExecutionEngine progExecEng(*p);

	ASSERT_NO_THROW(progExecEng.executeCurrentLine()) << "Execution of the first line of the program from Fixture should not fail.";
	progExecEng.next(); // Skips the intron automatically
	ASSERT_NO_THROW(progExecEng.executeCurrentLine()) << "Execution of the second line of the program from Fixture should not fail.";
	progExecEng.next();
	ASSERT_NO_THROW(progExecEng.executeCurrentLine()) << "Execution of the third line of the program from Fixture should not fail.";
	progExecEng.next();
	ASSERT_THROW(progExecEng.executeCurrentLine(), std::out_of_range) << "Execution of a non-existing line of the program should fail.";
}

TEST_F(ProgramExecutionEngineTest, setProgram) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Create a new program
	Program::Program p2(*e);

	ASSERT_NO_THROW(progExecEng.setProgram(p2)) << "Setting a new Program with a valid Environment for a ProgramExecutionEngine failed.";

	// Create a new incompatible program
	std::vector<std::reference_wrapper<const Data::DataHandler>> otherVect;
	otherVect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));
	Environment otherE(set, otherVect, 2);
	Program::Program p3(otherE);

	ASSERT_THROW(progExecEng.setProgram(p3), std::runtime_error) << "Setting a Program with an incompatible Environment should not be possible.";

	// Clean up
	delete& otherVect.at(0).get();
}

TEST_F(ProgramExecutionEngineTest, setDataSources) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Create a new compatible set of dataSources
	std::vector<std::reference_wrapper<const Data::DataHandler>> otherVect;
	otherVect.push_back(*vect.at(0).get().clone());
	otherVect.push_back(*vect.at(1).get().clone());

	ASSERT_NO_THROW(progExecEng.setDataSources(otherVect)) << "Setting a new valid set of Data Sources failed.";

	// Clean up
	delete& otherVect.at(0).get();
	delete& otherVect.at(1).get();
	otherVect.pop_back();
	otherVect.pop_back();

	// Create a new incompatible set of dataSources
	// although it has the same type and size of data, id of the
	// data handlers are different, which currently breaks the comparison.
	otherVect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
	otherVect.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

	ASSERT_THROW(progExecEng.setDataSources(otherVect), std::runtime_error) << "Setting a new invalid set of Data Sources should fail.";

	// Clean up
	delete& otherVect.at(0).get();
	delete& otherVect.at(1).get();
}

TEST_F(ProgramExecutionEngineTest, execute) {
	Program::ProgramExecutionEngine progExecEng(*p);
	double result;

	double r1 = value0 + 0;
	double r0 = r1 * (Parameter(value1)).operator float();
	r0 = r0 * value2 + r1 * value3;

	ASSERT_NO_THROW(result = progExecEng.executeProgram()) << "Program from fixture failed to execute. (Indivitual execution of its line in executeCurrentLine test).";
	ASSERT_EQ(result, r0) << "Result of the program from Fixture is not as expected.";

	// Introduce a new line in the program to test the throw
	Program::Line& l4 = p->addNewLine();
	// Instruction 3 does not exist. Must deactivate checks to write this instruction
	l4.setInstructionIndex(3, false);
	ASSERT_THROW(progExecEng.executeProgram(), std::out_of_range) << "Program line using a incorrect Instruction index should throw an exception.";

	// Now ignoring the exceptions
	ASSERT_NO_THROW(result = progExecEng.executeProgram(true)) << "Program line using a incorrect Instruction index should not interrupt the Execution when ignored.";
	ASSERT_EQ(result, r0) << "Result of the program from Fixture, with an additional ignored line, is not as expected.";
}
