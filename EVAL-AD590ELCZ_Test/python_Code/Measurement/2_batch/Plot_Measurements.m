%% Load csv files with impedence measurements
close all;
clear all;

plot_name = "2st Batch Measurements";

COM4 = importfile("COM4_Measurements.csv");  % Changed Cu electrode
COM5 = importfile("COM5_Measurements.csv");  % Continuous PANI electrode 
COM7 = importfile("COM7_Measurements.csv");  % Continuous Cu electrode 
COM8 = importfile("COM8_Measurements.csv");  % Changed PANI electrode

%% Plot COM4

% Take out the serie R from the value to have only the wanted value
COM4_Value = COM4(:,2)-1478;

figure(1);
grid on;
hold on;
title(plot_name)
xlabel('Points')
ylabel('Magnitude [Ohm]')

COM4_t = linspace(0,length(COM4),length(COM4));
plot(COM4_t,COM4_Value,'k','LineWidth',2);


%% Plot COM5

figure(2);
grid on;
hold on;
title(plot_name)
xlabel('Points')
ylabel('Magnitude [Ohm]')

% Take out the serie R from the value to have only the wanted value
COM5_Value = COM5(:,2)-1482;

COM5_t = linspace(0,length(COM5),length(COM5));
plot(COM5_t,COM5_Value,'b','LineWidth',2);


%% Plot COM7

figure(2);

% Take out the serie R from the value to have only the wanted value
COM7_Value = COM7(:,2)-1486;

COM7_t = linspace(0,length(COM7),length(COM7));
plot(COM7_t,COM7_Value,'g','LineWidth',2);

%% Plot COM8

figure(1);

% Take out the serie R from the value to have only the wanted value
COM8_Value = COM8(:,2)-1484;

COM8_t = linspace(0,length(COM8),length(COM8));
plot(COM8_t,COM8_Value,'m','LineWidth',2);

%% Add legends

figure(1);
legend('Cu Change','PANI Change','Location','NorthEastOutside')
figure(2);
legend('PANI Continued','Cu Continued','Location','NorthEastOutside')
