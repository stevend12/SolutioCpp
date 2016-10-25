% This file reads and plots the data from the PhotonDemo example of the solutio
% library.

fid = fopen("spectrums.txt", "r");
S = textscan(fid, "%f %f %f");
fclose(fid);

kV = 0:1:150;

figure
plot(kV, S{1}, "r")
hold on;
plot(kV, S{2}, "g")
plot(kV, S{3}, "b")
hold off;
legend("120 kVp", "120 kVp (3 mm Al)", "120 kVp (3 mm Cu)")
