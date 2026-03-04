%% Load csv files with impedence measurements
close all; clear all;

plot_name = "9st Batch Measurements";

COM6 = importfile("COM6_Measurements.csv");  % Changed Cu electrode
COM9 = importfile("COM9_Measurements.csv");  % Continuous PANI electrode 
COM8 = importfile("COM8_Measurements.csv");  % Continuous Cu electrode 
COM7 = importfile("COM7_Measurements.csv");  % Changed PANI electrode

%% Plot COM6

% Take out the serie R from the value to have only the wanted value
COM6_Value = COM6(:,2)-1478;

figure(1);
grid on;
hold on;
title(plot_name)
xlabel('Points')
ylabel('Magnitude [Ohm]')

COM6_t = linspace(0,length(COM6),length(COM6));
plot(COM6_t,COM6_Value,'k','LineWidth',2);


%% Plot COM9

figure(2);
grid on;
hold on;
title(plot_name)
xlabel('Points')
ylabel('Magnitude [Ohm]')

% Take out the serie R from the value to have only the wanted value
COM9_Value = COM9(:,2)-1482;

COM9_t = linspace(0,length(COM9),length(COM9));
plot(COM9_t,COM9_Value,'b','LineWidth',2);


%% Plot COM8

figure(2);

% Take out the serie R from the value to have only the wanted value
COM8_Value = COM8(:,2)-1486;

COM8_t = linspace(0,length(COM8),length(COM8));
plot(COM8_t,COM8_Value,'g','LineWidth',2);

%% Plot COM7

figure(1);

% Take out the serie R from the value to have only the wanted value
COM7_Value = COM7(:,2)-1484;

COM7_t = linspace(0,length(COM7),length(COM7));
plot(COM7_t,COM7_Value,'m','LineWidth',2);

%% Add legends

figure(1);
legend('Cu Change','PANI Change','Location','NorthEastOutside')
figure(2);
legend('PANI Continued','Cu Continued','Location','NorthEastOutside')
