% This file reads and plots the data from the DoseCalcDemo
% example of the SolutioCpp library.
clear;

% Read calculated data and create isodose lines
data = dlmread("tg43_isodose.txt");

X = zeros(size(data,1), size(data,2)-1);
Y = zeros(size(data,1), size(data,2)-1);
for r = 1:size(data, 1)
  for c = 1:(size(data,2)-1)
    X(r,c) = data(r,c+1) * cos(pi * data(r,1) / 180.0);
    Y(r,c) = data(r,c+1) * sin(pi * data(r,1) / 180.0);
  endfor
end
X = [X(2:end,:); flipud(X(2:end,:))];
Y = [Y(2:end,:); -1.*flipud(Y(2:end,:))];

% Rotate, scale, translate
rot_ang = 90.0 * (pi / 180.0);
X_R = X .* cos(rot_ang) - Y .* sin(rot_ang);
Y_R = X .* sin(rot_ang) + Y .* cos(rot_ang);

X_R = (X_R .* 128) + 290;
Y_R = (Y_R .* 128) + 290;

X_R(end+1,:) = X_R(1,:);
Y_R(end+1,:) = Y_R(1,:);

% Plot isodose lines on top of image
cmap = hot(size(data,2)-1);
close;

figure
hold on
for n = 1:size(X,2)
  plot(X_R(:,n), Y_R(:,n), '-', "color", cmap(n,:), "linewidth", 2.0)
end
rectangle("Position", [285 270 10 40], "Curvature", [0.2 0.2], ...
  "EdgeColor", [0.9 0.9 0.9], "FaceColor", [0.7 0.7 0.7])
hold off