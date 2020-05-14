% This file reads and plots the data from the PhysicsDemo example of the
% SolutioCpp library.

% Load photon attenuation data
fid = fopen("photon_data.txt", "r");
P = textscan(fid, "%f %f %f");
fclose(fid);

% Load photon attenuation data
fid = fopen("electron_data.txt", "r");
El = textscan(fid, "%f %f %f %f %f");
fclose(fid);

% Load TASMIP spectra
fid = fopen("spectrums.txt", "r");
S = textscan(fid, "%f %f %f");
fclose(fid);
kV = 0:1:150;

% Plot photon attenuation data
figure
loglog(P{1}, P{2}, "r")
hold on;
loglog(P{1}, P{3}, "b")
hold off;
axis([0.001 20 0.001 10000])
title("Photon Attenuation Coefficients")
xlabel("Photon Energy (MeV)")
ylabel("Mass Attenuation (cm^2 / g)")
legend("Lead", "Water")

% Plot electron attenuation data
figure
loglog(El{1}, El{2}, "r")
hold on;
loglog(El{1}, El{3}, "b")
loglog(El{1}, El{4}, "--g")
loglog(El{1}, El{5}, "--k")
hold off;
axis([0.1 100 0.001 100])
title("Electron Stopping Powers")
xlabel("Electron Energy (MeV)")
ylabel("Stopping Power (MeV cm^2 / g)")
legend("Lead (Col.)", "Lead (Rad.)", "Water (Col.)", "Water (Rad.)")

% Plot TASMIP spectra
figure
plot(kV, S{1}, "r")
hold on;
plot(kV, S{2}, "g")
plot(kV, S{3}, "b")
hold off;
axis([10 120 0 0.04])
title("TASMIP Spectra")
xlabel("Photon Energy (keV)")
legend("120 kVp", "120 kVp (3 mm Al)", "120 kVp (3 mm Cu)")

