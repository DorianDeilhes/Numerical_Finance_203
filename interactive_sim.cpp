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
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
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

string readLineOrThrow() {
  string line;
  if (!getline(cin, line)) {
    throw runtime_error("Input stream closed before the simulation was complete");
  }
  return line;
}

bool parseStrictInt(const string &line, int *value) {
  if (value == nullptr) {
    return false;
  }

  istringstream input(line);
  int parsed = 0;
  input >> parsed;
  if (!input) {
    return false;
  }

  string extra;
  if (input >> extra) {
    return false;
  }

  *value = parsed;
  return true;
}

bool parseStrictDouble(const string &line, double *value) {
  if (value == nullptr) {
    return false;
  }

  istringstream input(line);
  double parsed = 0.0;
  input >> parsed;
  if (!input || !isfinite(parsed)) {
    return false;
  }

  string extra;
  if (input >> extra) {
    return false;
  }

  *value = parsed;
  return true;
}

int getChoice(int lower, int upper) {
  int choice = 0;
  while (true) {
    const string line = readLineOrThrow();
    if (parseStrictInt(line, &choice) && choice >= lower && choice <= upper) {
      return choice;
    }
    cout << "Invalid input. Please enter an integer between "
         << lower << " and " << upper << ": ";
  }
}

int getIntInRange(const string &prompt, int lower, int upper) {
  int value = 0;
  while (true) {
    cout << prompt;
    const string line = readLineOrThrow();
    if (parseStrictInt(line, &value) && value >= lower && value <= upper) {
      return value;
    }
    cout << "Invalid input. Enter an integer between "
         << lower << " and " << upper << "." << endl;
  }
}

int getPositiveInt(const string &prompt) {
  return getIntInRange(prompt, 1, numeric_limits<int>::max());
}

double getFiniteDouble(const string &prompt) {
  double value = 0.0;
  while (true) {
    cout << prompt;
    const string line = readLineOrThrow();
    if (parseStrictDouble(line, &value)) {
      return value;
    }
    cout << "Invalid input. Enter one finite number." << endl;
  }
}

double getDoubleInRange(const string &prompt, double lower, double upper) {
  double value = 0.0;
  while (true) {
    cout << prompt;
    const string line = readLineOrThrow();
    if (parseStrictDouble(line, &value) && value >= lower && value <= upper) {
      return value;
    }
    cout << "Invalid input. Enter a finite number between "
         << lower << " and " << upper << "." << endl;
  }
}

double getPositiveDouble(const string &prompt) {
  double value = 0.0;
  while (true) {
    cout << prompt;
    const string line = readLineOrThrow();
    if (parseStrictDouble(line, &value) && value > 0.0) {
      return value;
    }
    cout << "Invalid input. Enter a finite number strictly greater than 0." << endl;
  }
}

double getNonNegativeDouble(const string &prompt) {
  double value = 0.0;
  while (true) {
    cout << prompt;
    const string line = readLineOrThrow();
    if (parseStrictDouble(line, &value) && value >= 0.0) {
      return value;
    }
    cout << "Invalid input. Enter a finite number greater than or equal to 0." << endl;
  }
}

int main() {
  try {
    cout << "============================================" << endl;
    cout << "   INTERACTIVE RANDOM NUMBER SIMULATOR      " << endl;
    cout << "============================================" << endl;

    // 1. Choose Base Uniform Generator
    vector<string> baseOptions = {"Linear Congruential (Standard)", "Ecuyer Combined (Advanced)"};
    printMenu("Choose Base Uniform Generator", baseOptions);
    int baseChoice = getChoice(1, 2);

    unique_ptr<UniformGenerator> uniformGen;
    if (baseChoice == 1) {
      cout << "Using default Linear Congruential parameters." << endl;
      uniformGen.reset(new LinearCongruential(42, 1103515245, 12345, 2147483648));
    } else {
      cout << "Using default Ecuyer Combined parameters." << endl;
      uniformGen.reset(new EcuyerCombined(12345, 67890));
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
    int numSamples = getPositiveInt("\nEnter number of samples to generate: ");

    cout << "\n--- GENERATING SAMPLES ---" << endl;

    if (distChoice == 1) {
      // HeadTail
      HeadTail gen(uniformGen.get());
      cout << "First " << min(10, numSamples) << " flips: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: 0.5)" << endl;

    } else if (distChoice == 2) {
      // Bernoulli
      double p = getDoubleInRange("Enter probability p (0 to 1): ", 0.0, 1.0);
      Bernoulli gen(p, uniformGen.get());
      cout << "First " << min(10, numSamples) << " outcomes: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << p << ")" << endl;

    } else if (distChoice == 3) {
      // Binomial
      int n = getPositiveInt("Enter number of trials n: ");
      double p = getDoubleInRange("Enter probability p (0 to 1): ", 0.0, 1.0);
      Binomial gen(n, p, uniformGen.get());
      cout << "First " << min(10, numSamples) << " combinations: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << n * p << ")" << endl;

    } else if (distChoice == 4) {
      // Poisson
      double lambda = getPositiveDouble("Enter lambda (> 0): ");
      cout << "1. First Algorithm (Multiplications)\n2. Second Algorithm (Sum of Exponentials)\nSelect algorithm: ";
      int algoChoice = getChoice(1, 2);
      PoissonAlgo algo = (algoChoice == 1) ? FirstAlgorithm : SecondAlgorithm;

      Poisson gen(lambda, algo, uniformGen.get());
      cout << "First " << min(10, numSamples) << " samples: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << lambda << ")" << endl;

    } else if (distChoice == 5) {
      // Exponential
      double lambda = getPositiveDouble("Enter lambda (> 0): ");
      Exponential gen(lambda, InverseDistribution, uniformGen.get());
      cout << "First " << min(10, numSamples) << " samples: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << 1.0 / lambda << ")" << endl;

    } else if (distChoice == 6) {
      // Normal
      double mu = getFiniteDouble("Enter mean (mu): ");
      double sigma = getNonNegativeDouble("Enter standard deviation (sigma >= 0): ");
      cout << "1. Box-Muller\n2. Central Limit Theorem\nSelect algorithm: ";
      int algoChoice = getChoice(1, 2);
      NormalAlgo algo = (algoChoice == 1) ? BoxMuller : CentralLimitTheorem;

      Normal gen(mu, sigma, algo, uniformGen.get());
      cout << "First " << min(10, numSamples) << " samples: ";
      for (int i = 0; i < min(10, numSamples); i++) cout << gen.Generate() << " ";
      cout << "\nEmpirical Mean (" << numSamples << " samples): " << gen.Mean(numSamples) << " (Expected: " << mu << ")" << endl;

    } else if (distChoice == 7) {
      // Bivariate Normal
      double mu_x = getFiniteDouble("Enter mean X (mu_x): ");
      double mu_y = getFiniteDouble("Enter mean Y (mu_y): ");
      double sig_x = getPositiveDouble("Enter std dev X (sigma_x > 0): ");
      double sig_y = getPositiveDouble("Enter std dev Y (sigma_y > 0): ");
      double rho = getDoubleInRange("Enter correlation (rho between -1 and 1): ", -1.0, 1.0);

      BivariateNormal gen(mu_x, mu_y, sig_x, sig_y, rho, BoxMuller, uniformGen.get());
      cout << "First " << min(5, numSamples) << " pairs (X, Y): " << endl;
      for (int i = 0; i < min(5, numSamples); i++) {
        pair<double, double> p = gen.Generate();
        cout << "  (" << p.first << ", " << p.second << ")" << endl;
      }
      cout << "\nEmpirical Mean X (" << numSamples << " samples): " << gen.MeanFirst(numSamples) << " (Expected: " << mu_x << ")" << endl;
      cout << "Empirical Mean Y (" << numSamples << " samples): " << gen.MeanSecond(numSamples) << " (Expected: " << mu_y << ")" << endl;
      cout << "Empirical Correlation (" << numSamples << " samples): " << gen.Correlation(numSamples) << " (Expected: " << rho << ")" << endl;
    }

    cout << "\nSimulation Complete. Thank you for using the Interactive Simulator!" << endl;
  } catch (const exception &exception) {
    cerr << "\nInteractive simulator stopped: " << exception.what() << endl;
    return 1;
  }

  return 0;
}
