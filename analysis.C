#include <iostream>
#include <fstream>
#include <vector>
#include <TString.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <algorithm>
#include <TH1D.h>
#include <TF1.h>
#include <TStyle.h>



// Calculate the weighted mean for the x-coordinate
Double_t calculateWeightedMeanX(double Px1, double Nx1, double sigmax1) {
    if (sigmax1 == 0 ) {
        std::cout << "Error: Sigma values cannot be zero!" << endl;
        return -1; // Return a sentinel value to indicate error
    }
    return (Px1 * Nx1 / sigmax1 ) / (Nx1 / sigmax1 );
}

// Calculate the weighted mean for the y-coordinate
Double_t calculateWeightedMeanY(double Py1, double Py2, double Ny1, double Ny2, double sigmay1, double sigmay2) {
    if (sigmay1 == 0 || sigmay2 == 0) {
        std::cout << "Error: Sigma values cannot be zero!" << endl;
        return -1; // Return a sentinel value to indicate error
    }
    return (Py1 * Ny1 / sigmay1 + Py2 * Ny2 / sigmay2) / (Ny1 / sigmay1 + Ny2 / sigmay2);
}



void analysis() {
    TString dir = "/home/harriet/Geant4/neutron_example/OPPAC_Sim-master/build";
    std::vector<TString> files{"photon_positions_1.csv"};//,"photon_positions_2.csv", "photon_positions_3.csv", "photon_positions_4.csv"};

    
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);   
    
    TCanvas *c1 = new TCanvas("c1", "Sensor Numbers Plots", 800, 600);
    c1->Divide(2, 2); // Divide canvas into 4 subpads
 
    TCanvas *c2 = new TCanvas("c2", "Histograms with Gaussian Fit", 800, 600);
    c2->Divide(5, 5); // Canvas for histograms with Gaussian fits


    TCanvas *c3 = new TCanvas("c3", "Selected Points", 800, 600); 
    TH1D *xPosHist = new TH1D("xPosHist", "Y Position", 50, -50, 50); 

    TCanvas *c4 = new TCanvas("c4", "X Position Distribution", 800, 600);
    TH1D *histXPos = new TH1D("histXPos", "Distribution of X Position", 0, 0, 40);

    int pad = 1;
    int pad1 = 1;
    for (auto iFile : files) {
        TString fullPath = dir + "/" + iFile;

        std::ifstream file(fullPath.Data());
        if (!file.is_open()) {
            std::cerr << "Error: File " << fullPath << " does not exist or could not be opened." << std::endl;
            continue;
        }

        std::vector<double> column1;
        std::vector<double> column2;
        std::vector<double> column4;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string col1, col2, col3,col4;

            
            std::getline(ss, col1, ',');
            std::getline(ss, col2, ',');
            std::getline(ss, col3, ',');
            std::getline(ss,col4, ',');

           if (!col1.empty() && !col2.empty() && !col4.empty()) {
              column1.push_back(std::stod(col1)); // X values
              column2.push_back(std::stod(col2)); // Y values
              column4.push_back(std::stod(col4)); // Sensor values
           }
        }

        file.close();

        
        double min = *std::min_element(column4.begin(), column4.end());
        double max = *std::max_element(column4.begin(), column4.end());
        int bins = 50; 

        TH1D *hist = new TH1D(iFile.Data(), ("Histogram of " + iFile).Data(), bins, min, max);
        for (double value : column4) {
            hist->Fill(value);
        }

        c1->cd(pad++); 
        hist->GetXaxis()->SetTitle("Sensor Number");
        hist->GetYaxis()->SetTitle("Counts");
        hist->SetLineColor(kBlue);
        hist->Draw();
    

        // Fit the histogram with a Gaussian
        TF1 *gaussFit1 = new TF1("gaussFit", "gaus", min, max);
        gaussFit1->SetLineColor(kRed);
        hist->Fit(gaussFit1, "R");

        // Extract Gaussian fit parameters
        double mean = gaussFit1->GetParameter(1);      // Mean (μ)
        double sigma = gaussFit1->GetParameter(2);     // Standard deviation (σ)
        double fwhm = 2.355 * sigma;                  // Full Width at Half Maximum (FWHM)

  /*      // Print fit results to the console
        std::cout << "File: " << iFile << std::endl;
        std::cout << "  Mean (μ): " << mean << std::endl;
        std::cout << "  Std Dev (σ): " << sigma << std::endl;
        std::cout << "  FWHM: " << fwhm << std::endl;
        std::cout << "  Total Events: " << column4.size() << std::endl;
*/
        
      
        std::vector<double> xPositions; // Vector to store x_pos values       
        for (int valueToCheck = 0; valueToCheck <= 32; ++valueToCheck) {
            // Vectors to hold x and y coordinates for the matching values
            std::vector<double> matchingX;
            std::vector<double> matchingY;
             std::vector<double> matchingV;

            // Iterate through column4 and check for matching values
            for (size_t i = 0; i < column4.size(); ++i) {
                if (static_cast<int>(column4[i]) == valueToCheck) { // Compare to valueToCheck
                   matchingX.push_back(column1[i]); // Add corresponding x value
                   matchingY.push_back(column2[i]); // Add corresponding y value
                   matchingV.push_back(column4[i]);
                }
            }

            // If no matching values, skip this iteration
            if (matchingX.empty()) continue;

           /* // Print or process the matching values
            std::cout << "Value " << valueToCheck << " matches " << matchingX.size() << " entries." << std::endl;
            for (size_t j = 0; j < matchingX.size(); ++j) {
                std::cout << "x: " << matchingX[j] << ", y: " << matchingY[j] << std::endl;
            }*/

     
            // Create a histogram for the current value
            int bins = 10; 
            double min = *std::min_element(matchingY.begin(), matchingY.end());
            double max = *std::max_element(matchingY.begin(), matchingY.end());
            TH1D *hist1 = new TH1D(Form("hist_%d", valueToCheck),Form("Histogram for value %d", valueToCheck),bins, min, max);

            // Fill the histogram
            for (double val : matchingY) {
                hist1->Fill(val);
            }
            c2->cd(pad++); 
           hist1->GetXaxis()->SetTitle(" Y_Position [mm]");
           hist1->GetYaxis()->SetTitle("Counts");
           hist1->SetLineColor(kBlue);
           hist1->Draw();


            // Fit the histogram with a Gaussian
           TF1 *gaussFit = new TF1("gaussFit", "gaus", min, max);
           hist1->Fit(gaussFit, "R");
        
            // Extract Gaussian fit parameters
           double mean1 = gaussFit->GetParameter(1);      // Mean (centroid)
           double sigma1 = gaussFit->GetParameter(2);     // Standard deviation
           double amplitude1 = gaussFit->GetParameter(0); // Amplitude (height of the peak)

            // Calculate x_position
            double x_pos = calculateWeightedMeanX( mean1, amplitude1, sigma1);

        
           
            // Print results
            std::cout << "File: " << iFile << ", Value: " << valueToCheck
                  << " | Mean: " << mean1 << " | Sigma: " << sigma1
                  << " | Amplitude: " << amplitude1
                  << " | x_pos: " << x_pos << std::endl;

            // Store x_pos in the vector
            //xPositions.push_back(x_pos);
 
           xPosHist->Fill(x_pos);

        
       


       }
     
// Draw the histogram
       c3->cd();
       xPosHist->GetXaxis()->SetTitle("Y_recontruction [mm]");
       xPosHist->GetYaxis()->SetTitle("Counts");
       xPosHist->SetMarkerStyle(20); 
       xPosHist->Draw("P"); 
        
    }

    c1->Update();
    c2->Update();
    //c3->Update();
}
