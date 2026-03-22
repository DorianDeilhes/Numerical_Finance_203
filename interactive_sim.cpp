#include "ContinuousGenerator/BivariateNormal.h"
#include "ContinuousGenerator/Exponential.h"
#include "ContinuousGenerator/Normal.h"
#include "DiscreteGenerator/Bernoulli.h"
#include "DiscreteGenerator/Binomial.h"
#include "DiscreteGenerator/FiniteSet.h"
#include "DiscreteGenerator/HeadTail.h"
#include "DiscreteGenerator/Poisson.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/LinearCongruential.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void printMenu(const string &title, const vector<string> &options) {
  cout << "\n=== " << title << " ===" << endl;
  for (size_t i = 0; i < options.size(); ++i) {
    cout << i + 1 << ". " << options[i] << endl;
  }
  cout << "Select an option (1-" << options.size() << "): ";
}

int getChoice(int min, int max) {
  int choice;
  while (true) {
    if (cin >> choice && choice >= min && choice <= max) {
      return choice;
    }
    cout << "Invalid input. Please enter a number between " << min << " and " << max << ": ";
    cin.clear();
    cin.ignore(10000, '\n');
  }
}

double getDoubleInput(const string &prompt) {
  double val;
  cout << prompt;
  while (!(cin >> val)) {
    cout << "Invalid input. " << prompt;
    cin.clear();
    cin.ignore(10000, '\n');
  }
  return val;
}

int getIntInput(const string &prompt) {
  int val;
  cout << prompt;
  while (!(cin >> val)) {
    cout << "Invalid input. " << prompt;
    cin.clear();
    cin.ignore(10000, '\n');
  }
  return val;
}

int main() {
  cout << "============================================" << endl;
  cout << "   INTERACTIVE RANDOM NUMBER SIMULATOR      " << endl;
  cout << "============================================" << endl;

  // 1. Choose Base Uniform Generator
  vector<string> baseOptions = {"Linear Congruential (Standard)", "Ecuyer Combined (Advanced)"};
  printMenu("Choose Base Uniform Generator", baseOptions);
  int baseChoice = getChoice(1, 2);

  UniformGenerator *uniformGen = nullptr;
  if (baseChoice == 1) {
    cout << "Using default Linear Congruential parameters." << endl;
    uniformGen = new LinearCongruential(42, 1103515245, 12345, 2147483648);
  } else {
    cout << "Using default Ecuyer Combined parameters." << endl;
    uniformGen = new EcuyerCombined(12345, 67890);
  }

  // 2. Choose Distribution Type
  vector<string> distOptions = {
      "Head/Tail (Coin Flip)",
      "Bernoulli",
      "Binomial",
      "Poisson",
      "Exponential",
      "Normal",
      "Bivariate Normal"
  };
  printMenu("Choose Distribution to Simulate", distOptions);
  int distChoice = getChoice(1, 7);

  // 3. Number of samples
  int numSamples = getIntInput("\nEnter number of samples to generate: ");
  if (numSamples <= 0) numSamples = 1;

  cout << "\n--- GENERATING SAMPLES ---" << endl;

  if (distChoice == 1) {
    // HeadTail
    HeadTail gen(uniformGen);
    cout << "First " << min(10, numSamples) << " flips: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: 0.5)" << endl;

  } else if (distChoice == 2) {
    // Bernoulli
    double p = getDoubleInput("Enter probability p (0 to 1): ");
    Bernoulli gen(p, uniformGen);
    cout << "First " << min(10, numSamples) << " outcomes: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << p << ")" << endl;

  } else if (distChoice == 3) {
    // Binomial
    int n = getIntInput("Enter number of trials n: ");
    double p = getDoubleInput("Enter probability p: ");
    Binomial gen(n, p, uniformGen);
    cout << "First " << min(10, numSamples) << " combinations: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << n * p << ")" << endl;

  } else if (distChoice == 4) {
    // Poisson
    double lambda = getDoubleInput("Enter lambda (> 0): ");
    cout << "1. First Algorithm (Multiplications)\n2. Second Algorithm (Sum of Exponentials)\nSelect algorithm: ";
    int algoChoice = getChoice(1, 2);
    PoissonAlgo algo = (algoChoice == 1) ? FirstAlgorithm : SecondAlgorithm;
    
    Poisson gen(lambda, algo, uniformGen);
    cout << "First " << min(10, numSamples) << " samples: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << lambda << ")" << endl;

  } else if (distChoice == 5) {
    // Exponential
    double lambda = getDoubleInput("Enter lambda (> 0): ");
    Exponential gen(lambda, InverseDistribution, uniformGen);
    cout << "First " << min(10, numSamples) << " samples: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << 1.0 / lambda << ")" << endl;

  } else if (distChoice == 6) {
    // Normal
    double mu = getDoubleInput("Enter mean (mu): ");
    double sigma = getDoubleInput("Enter standard deviation (sigma): ");
    cout << "1. Box-Muller\n2. Central Limit Theorem\nSelect algorithm: ";
    int algoChoice = getChoice(1, 2);
    NormalAlgo algo = (algoChoice == 1) ? BoxMuller : CentralLimitTheorem;
    
    Normal gen(mu, sigma, algo, uniformGen);
    cout << "First " << min(10, numSamples) << " samples: ";
    for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
    cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << mu << ")" << endl;
    
  } else if (distChoice == 7) {
    // Bivariate Normal
    double mu_x = getDoubleInput("Enter mean X (mu_x): ");
    double mu_y = getDoubleInput("Enter mean Y (mu_y): ");
    double sig_x = getDoubleInput("Enter std dev X (sigma_x): ");
    double sig_y = getDoubleInput("Enter std dev Y (sigma_y): ");
    double rho = getDoubleInput("Enter correlation (rho between -1 and 1): ");
    
    BivariateNormal gen(mu_x, mu_y, sig_x, sig_y, rho, BoxMuller, uniformGen);
    cout << "First " << min(5, numSamples) << " pairs (X, Y): " << endl;
    for (int i = 0; i < min(5, numSamples); i++) {
      pair<double, double> p = gen.Generate();
      cout << "  (" << p.first << ", " << p.second << ")" << endl;
    }
    cout << "\nEmpirical Mean X (" << numSamples << " samples): " << gen.MeanFirst(numSamples) << " (Expected: " << mu_x << ")" << endl;
    cout << "Empirical Mean Y (" << numSamples << " samples): " << gen.MeanSecond(numSamples) << " (Expected: " << mu_y << ")" << endl;
    cout << "Empirical Correlation (" << numSamples << " samples): " << gen.Correlation(numSamples) << " (Expected: " << rho << ")" << endl;
  }

  delete uniformGen;
  cout << "\nSimulation Complete. Thank you for using the Interactive Simulator!" << endl;

  return 0;
}
