% 1-D Array for ultrasonic-phased array
% date: 1 April 2016
%
% This file is used for a simulation of an array of low frequency
% ultrasonic trancducers, it builds on the simulating transducer
% field patterns example in k-wave, which is part of the k-Wave Toolbox (http://www.k-wave.org)
% Copyright (C) 2009-2014 Bradley Treeby and Ben Cox

clear all;

% =========================================================================
% SIMULATION
% =========================================================================

% create the computational grid
Nx = 2*560;           % number of grid points in the x (row) direction
Ny = 2*420;            % number of grid points in the y (column) direction
dx = 0.0009;    	% grid point spacing in the x direction [m]
dy = dx;            % grid point spacing in the y direction [m]
kgrid = makeGrid(Nx, dx, Ny, dy);

% define the properties of the propagation medium
medium.sound_speed = 343;  % [m/s]
medium.alpha_coeff = 6.2256181855618e-01;% [dB/(MHz^y cm)] %% 6.2256181855618e-01
medium.alpha_power = 1.2211289052168; 

% create the time array
[kgrid.t_array, dt] = makeTime(kgrid, medium.sound_speed);
kgrid.t_array = 0:dt:2*2500*dt;

% define source mask for a linear transducer with an odd number of elements
spacing_factor = 6;
num_elements = 8;      % [grid points]
x_offset = 10;          % [grid points]
source.p_mask = zeros(Nx, Ny);
start_index = Ny/2 - round(num_elements/2*spacing_factor) + 1;
source.p_mask(x_offset, start_index:spacing_factor:start_index + spacing_factor*num_elements - 1) = 1;

% define the properties of the tone burst used to drive the transducer
sampling_freq = 1/dt;   % [Hz]
steering_angle = 0;    % [deg]
element_spacing = spacing_factor * dx;   % [m]
tone_burst_freq = 0.04e6;  % [Hz]
tone_burst_cycles = 25;

% create an element index relative to the centre element of the transducer
element_index = -(num_elements - 1)/2:(num_elements - 1)/2;
% use geometric beam forming to calculate the tone burst offsets for each
% transducer element based on the element index
tone_burst_offset = 40 + element_spacing*element_index*sin(steering_angle*pi/180)/(medium.sound_speed*dt);
% create the tone burst signals
source_strength=20;
source.p = source_strength*toneBurst(sampling_freq, tone_burst_freq, tone_burst_cycles, 'SignalOffset', tone_burst_offset);

% Define Sensor Mask
sensor.mask = ones(Nx, Ny);
sensor.record = {'p_rms', 'p_max'};

% assign the input options
input_args = {'DisplayMask', source.p_mask, 'RecordMovie', true, 'MovieName', 'film', 'MovieType', 'frame', 'PlotFreq', 5, 'MovieArgs', {'fps', 30}};

% run the simulation
sensor_data = kspaceFirstOrder2D(kgrid, medium, source,sensor, input_args{:},'PlotScale', [-source_strength/8, source_strength/8]);

% =========================================================================
% VISUALISATION
% =========================================================================


% Plot redcorded pressure
sensor_data.p_max = reshape(sensor_data.p_max, [Nx, Ny]);
figure;
imagesc(kgrid.y_vec*1e3, (kgrid.x_vec - min(kgrid.x_vec(:)))*1e3, (sensor_data.p_max),[0 3]);
xlabel('y-position [mm]');
ylabel('x-position [mm]');
title('Total Beam Pattern Using Maximum Of Recorded Pressure');
colormap(jet(256));
c = colorbar;
ylabel(c, 'Pressure [Pa]');
axis image;
    
% % plot the input time series
% figure;
% num_source_time_points = length(source.p(1,:));
% [t_sc, scale, prefix] = scaleSI(kgrid.t_array(num_source_time_points));
% stackedPlot(kgrid.t_array(1:num_source_time_points)*scale, source.p);
% xlabel(['Time [' prefix 's]']);
% ylabel('Input Signals');