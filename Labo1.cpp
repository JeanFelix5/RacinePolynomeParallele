// exemple-envoi-vecteur.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
/******************************************************************************
	Hello World example:

	In this example the rank 0 of MPI_COMM_WORLD waits for messages from the
	other ranks. When a message is received it is displayed (send to stdout).

	Copyright 2003 (c) Critical Software SA
	. http://www.criticalsoftware.com
	. http://www.criticalsoftware.com/hpc
	. wmpi@criticalsoftware.com
 *****************************************************************************/



//Code de Jean-Félix Laboratoire #1

#include "pch.h"

#include <mpi.h>
#include <vector>
#include <iomanip>  
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <algorithm>


#define MESSAGELENGTH  128  /* The length of the messages that               */
							/* we are going to transmit                      */
#define TAG            100  /* The tag to use for the transmissions          */

using namespace std;

int				    TotalRootFoundVector = 0;
int                 sumRoots = 0;
string				PolynomeCommandLine = "";
vector<string>		PolynomeVec;
vector<double>      RootFoundVector;


bool compare_double(double x, double y, double epsilon = 0.01f) {
	if (fabs(x - y) < epsilon) {
		return true; //they are same
	}
	return false; //they are not same
}

void FonctionTest(double n_intervalTotal, double n_smallInterval, double startInterval, double endInterval, int idProcesses) {
	
	double Width = 1.0 / n_intervalTotal;
	double polynome = 0.0;
	double start = startInterval;
	double end = endInterval;
	double x = 0.0;

	for (double i = start; i <= end; i += Width) {

		x = i;

		if (PolynomeCommandLine != "") {	//polynome dans la ligne de commande (BONUS)
	
			for (int i = 0; i < PolynomeVec.size(); i++) {

				if (PolynomeVec.at(i) == "*") {
					i++;
				}

				PolynomeCommandLine = PolynomeVec.at(i);

				string varx("x");
				int pos;

				while ((pos = PolynomeCommandLine.find(varx)) != string::npos) {

					PolynomeCommandLine.replace(pos, varx.length(), "");
				}

				string parenthese1("(");
				int pos1;

				while ((pos1 = PolynomeCommandLine.find(parenthese1)) != string::npos) {

					PolynomeCommandLine.replace(pos1, parenthese1.length(), "");
				}

				string parenthese2(")");
				int pos2;

				while ((pos2 = PolynomeCommandLine.find(parenthese2)) != string::npos) {

					PolynomeCommandLine.replace(pos2, parenthese2.length(), "");
				}

				polynome = stod(PolynomeCommandLine.c_str());
				polynome = x + polynome;

				if (compare_double(polynome, 0) == true) {
					RootFoundVector.push_back(x);
					TotalRootFoundVector++;

					cout << "Le processus -> " << idProcesses << " a trouver la racine suivante : " << x
						<< " se trouvant de l'interval -> " << start << " a " << end << endl;
				}
			}
							
		}
		else {	//pas de polynome dans la ligne de commande (donc polynome hardcoder)
			polynome = (x - 0.1) * (x - 0.5) * (x - 0.9) * (x - 5.3) * (x - 8.0);


			if (compare_double(polynome, 0) == true) {

				RootFoundVector.push_back(x);
				TotalRootFoundVector++;

				cout << "Le processus -> " << idProcesses << " a trouver la racine suivante : " << x
					<< " se trouvant de l'interval -> " << start << " a " << end << endl;
			}
		}

		if (x > 10) {//x ne doit pas dépasser 10
			break;
		}
	}
}

/******************************************************************************
	main:
 *****************************************************************************/
int main(int argc, char** argv)
{
	int         nCommSize;
	int         nCommRank;
	double		nIntervals = 0.0;
	int         nCounter = 0;
	double		vector1[MESSAGELENGTH];
	char    pchNodeName[MPI_MAX_PROCESSOR_NAME];
	int		nNodeNameSize;

	/* Initialize WMPI II:                                                   */
	MPI_Init(&argc, &argv);

	/* Determine what the world looks like and our own position in it:       */
	MPI_Comm_size(MPI_COMM_WORLD, &nCommSize); //get the number of processes (8)
	MPI_Comm_rank(MPI_COMM_WORLD, &nCommRank); //get the number of the current process (exemple: 3/8)

	stringstream ss;
	ss << "Processor " << nCommRank << " receives argc = " << argc << " with argv = " << argv[0] << " ";
	for (int i = 1; i < argc; i++) {
		ss << argv[i];
		PolynomeCommandLine += argv[i];
		PolynomeVec.push_back(argv[i]);	
	}
	
	/* If I am rank 0 I'll receive messages from the other ranks in          */
	/* MPI_COMM_WORLD and send them to stdout:                               */
	if (nCommRank == 0) {
		cout << '\n' << ss.str() << '\n' << '\n';

		cout << "Programmation parallele - Lab #1!\n";
		cout << "NOTE: Si vous entrez un polynome en ligne de commande, veuillez mettre un espace entre le ( * ) Sinon cela ne fonctionnera pas. Exemple: (x-3.4) * (x-8.3) * (x-1.1)" << endl << endl;
		
		if (PolynomeCommandLine == "") {
			cout << "Le polynome dont l'on souhaite trouver les racine est: " << "(x - 0.1) * (x - 0.5) * (x - 0.9) * (x - 5.3) * (x - 8.0)" << endl;
		}
		else {
			cout << "Le polynome dont l'on souhaite trouver les racine est: " << PolynomeCommandLine << endl;
		}
		cout << "Interval est de 0 a (normalement 10): ";
		cin >> nIntervals;
		
		for (nCounter = 1; nCounter < nCommSize; nCounter++)
			MPI_Send(&nIntervals, 1, MPI_DOUBLE, nCounter, 98, MPI_COMM_WORLD);
	}
	else /* I am NOT rank 0 */ {

		/* If I am a Slave just wait for the Master to tell me how many intervals we are going compute:          */
		MPI_Recv(&nIntervals,
			1,
			MPI_DOUBLE,
			0,
			98,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
	}
	
	/* Get my node name  */
	MPI_Get_processor_name(pchNodeName, &nNodeNameSize);

	//je m'assure que l'interval est valide pour le polynome fournis
	if (nIntervals > 10 || nIntervals < 0) {
		nIntervals = 10;
	}

	nIntervals = nIntervals / nCommSize;

	/* If I am the Master, I should collect results, compute the sum, and display the result: */
	if (nCommRank == 0) {	//processus 0

		FonctionTest(20, nIntervals, 0.0, nIntervals, nCommRank);//mets un interval de 20, car 1/20 = 0,05 et je verifie a tous les 0,05 s'il y a une racine
		sumRoots = TotalRootFoundVector;

		//msg1
		MPI_Recv(&TotalRootFoundVector,
			1,
			MPI_INT,
			1,
			98,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);

		sumRoots += TotalRootFoundVector;

		if (TotalRootFoundVector != 0) {
			//msg2
			MPI_Recv(vector1,
				MESSAGELENGTH,
				MPI_DOUBLE,
				1,
				99,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
		}

		for (int i = 0; i < TotalRootFoundVector; i++) {
			RootFoundVector.push_back(vector1[i]);
		}
		
		//msg1
		MPI_Recv(&TotalRootFoundVector,
			1,
			MPI_INT,
			2,
			98,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);

		sumRoots += TotalRootFoundVector;
		
		if (TotalRootFoundVector != 0) {
			//msg2
			MPI_Recv(&vector1,
				MESSAGELENGTH,
				MPI_DOUBLE,
				2,
				99,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
		}

		for (int i = 0; i < TotalRootFoundVector; i++) {
			RootFoundVector.push_back(vector1[i]);
		}

		//Étape 3 : 0 <= 4
		//msg1
		MPI_Recv(&TotalRootFoundVector,
			1,
			MPI_INT,
			4,
			98,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);

		if (TotalRootFoundVector != 0) {
			//msg2
			MPI_Recv(vector1,
				MESSAGELENGTH,
				MPI_DOUBLE,
				4,
				99,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
		}

		for (int i = 0; i < TotalRootFoundVector; i++) {
			RootFoundVector.push_back(vector1[i]);
		}

		sumRoots += TotalRootFoundVector;
		
		TotalRootFoundVector = sumRoots;
		cout << "Nombre de racine trouve au total: " << TotalRootFoundVector << endl;

		for (int i = 0; i < RootFoundVector.size(); i++) {
			vector1[i] = RootFoundVector.at(i);
		}

		cout << endl << "Le processus 0 a recu les racines suivantes: " << endl;
		for (int i = 0; i < TotalRootFoundVector; i++) {
			cout << RootFoundVector.at(i) << endl;
		}
	}
	else {	//tous les autres processus (n'inclue pas 0)
		/* If I am a Slave */
		double startInterval = 0.0;
		for (nCounter = 1; nCounter <= nCommSize; nCounter++) {
			startInterval += nIntervals;

			if (nCounter == nCommRank) {
				FonctionTest(20, nIntervals, startInterval, startInterval + nIntervals, nCommRank); //mets un interval de 20, car 1/20 = 0,05 et je verifie a tous les 0,05 s'il y a une racine
				break;
			}
		}

		for (int i = 0; i < RootFoundVector.size(); i++) {	//put the roots that were found with the process in the vector to send it after
			vector1[i] = RootFoundVector.at(i);
		}

		//Étape 1 : 0 <= 1 2 <= 3 4 <= 5 6 <= 7
		for (nCounter = 1; nCounter <= nCommSize; nCounter++) {
			if (nCommRank % 2 == 0) {//pair
				sumRoots = RootFoundVector.size();

				//msg1
				MPI_Recv(&TotalRootFoundVector,
					1,
					MPI_INT,
					nCommRank + 1,
					98,
					MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);

				sumRoots += TotalRootFoundVector;

				if (TotalRootFoundVector != 0) {
					//msg2
					MPI_Recv(vector1,
						MESSAGELENGTH,
						MPI_DOUBLE,
						nCommRank + 1,
						99,
						MPI_COMM_WORLD,
						MPI_STATUS_IGNORE);
				}

				for (int i = 0; i < TotalRootFoundVector; i++) {
					RootFoundVector.push_back(vector1[i]);
				}

				TotalRootFoundVector = sumRoots;

				for (int i = 0; i < RootFoundVector.size(); i++) {
					vector1[i] = RootFoundVector.at(i);
				}
			}
			else {//impair
				//send number of root found (message 1)
				MPI_Send(&TotalRootFoundVector, 1, MPI_INT, nCommRank - 1, 98, MPI_COMM_WORLD);

				if (TotalRootFoundVector != 0) {
					//send the roots that were found (message 2)                                      
					MPI_Send(vector1, MESSAGELENGTH, MPI_DOUBLE, nCommRank - 1, 99, MPI_COMM_WORLD);
				}
			}
		}

		TotalRootFoundVector = RootFoundVector.size();


		//Étape 2 : 0 <= 2 4 <= 6
		switch (nCommRank) {
			case 2:
				if (RootFoundVector.size() == 0) {
					//send number of root found (message 1)
					MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 0, 98, MPI_COMM_WORLD);
					//cout << "No roots were found for the process -> " << nCommRank << endl;
					break;
				}
				//send number of root found (message 1)
				MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 0, 98, MPI_COMM_WORLD);

				//send the roots that were found (message 2)                                      
				MPI_Send(vector1, MESSAGELENGTH, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD);
				break;

			case 4:
				//msg1
				MPI_Recv(&TotalRootFoundVector,
					MESSAGELENGTH,
					MPI_INT,
					6,
					98,
					MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);

				sumRoots += TotalRootFoundVector;

				if (TotalRootFoundVector != 0) {
					//msg2
					MPI_Recv(vector1,
						MESSAGELENGTH,
						MPI_DOUBLE,
						6,
						99,
						MPI_COMM_WORLD,
						MPI_STATUS_IGNORE);
				}

				for (int i = 0; i < TotalRootFoundVector; i++) {
					RootFoundVector.push_back(vector1[i]);
				}

				TotalRootFoundVector = sumRoots;

				if (RootFoundVector.size() == 0) {
					//send number of root found (message 1)
					MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 0, 98, MPI_COMM_WORLD);
					//cout << "No roots were found for the process -> " << nCommRank << endl;
					break;
				}

				for (int i = 0; i < RootFoundVector.size(); i++) {
					vector1[i] = RootFoundVector.at(i);
				}

				//send number of root found (message 1)
				MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 0, 98, MPI_COMM_WORLD);

				//send the roots that were found (message 2)                                      
				MPI_Send(vector1, MESSAGELENGTH, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD);
				break;

			case 6:

				if (RootFoundVector.size() == 0) {
					//send number of root found (message 1)
					MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 4, 98, MPI_COMM_WORLD);
					//cout << "No roots were found for the process -> " << nCommRank << endl;
					break;
				}
				//send number of root found (message 1)
				MPI_Send(&TotalRootFoundVector, 1, MPI_INT, 4, 98, MPI_COMM_WORLD);

				//send the roots that were found (message 2)                                      
				MPI_Send(vector1, MESSAGELENGTH, MPI_DOUBLE, 4, 99, MPI_COMM_WORLD);
				break;

			default:
				break;
		}
	}
		
	/* Flush the output so it's shown when using mpiexec                     */
	//fflush(stdout);

	/* Pause rank 0 so that the output can be verified:                      */
	if (nCommRank == 0) {
		cout << "\nPress ENTER to exit...\n" << endl;
		//fflush(stdout);
		getchar();
	}

	/* Finalize WMPI II:                                                     */
	MPI_Finalize();
	return 0;
}

//mpiexec -n 8 Labo1.exe <- the command to type in the windows command panel
