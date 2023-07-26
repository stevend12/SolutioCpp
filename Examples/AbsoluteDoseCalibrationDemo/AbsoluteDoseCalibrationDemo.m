% This file reads and plots the data from the AbsoluteDoseCalibrationDemo
% example of the SolutioCpp library.
clear;

% Load kQ data and calculate difference
kQ = dlmread("k_Q.txt");
kQ_PD = 100.0 .* ((kQ(:,2) - kQ(:,3)) ./ kQ(:,3));
printf("k_Q Average Difference: %.02f\n", mean(kQ_PD));
printf("k_Q Max. Difference: %.02f\n", max(kQ_PD));

% Load k_R50_prime
kR50_axis = 2:9;
kR50 = dlmread("k_R50_prime.txt");

% Load k_ecal data
k_ecal = dlmread("k_ecal.txt");

figure
plot(kQ(:,1),kQ(:,2:3))
title("Exradin A19 k_{Q}")
xlabel("PDD (10 cm depth)")
ylabel("k_{Q}")
legend("SolutioCpp", "TG-51 Addendum")

figure
plot(kR50_axis, kR50)
title("k'_{R50}")
xlabel("R_{50}")
ylabel("k'_{R50}")
legend("Exradin A12", "IBA CC13", "NE2561", "PTW 30013", "TG-51 Fit")

figure
bar_labels = {"Exradin A12", "IBA CC13", "NE2561", "PTW 30013"};
bar(k_ecal)
title("k_{ecal}")
axis([0 5 0.85 0.92])
set(gca, 'XTickLabel', bar_labels)
ylabel("k_{ecal}")
legend("SolutioCpp", "TG-51 Table")