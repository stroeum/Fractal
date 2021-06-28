close all
clear all
clc
beep  off

%% Load relevant data %%
%-------------------------------------------------------------------------%
% Load data files %
%-------------------------------------------------------------------------%

dxyz = load('dxyz.dat')*1e-3; %_km
Nxyz = load('Nxyz.dat'); %_dimensionless
InitPoint = load('InitPoint.dat')*1e-3; %_km
Links.data = load('EstablishedLinks.dat');
z_gnd = load('z_gnd.dat')*1e-3; %_km
ChargeLayers.data = load('ChargeLayers.dat')*1e-3; %_kC, _km
rho.YZ = load('rhoAmbYZ.dat');
rho.XZ = load('rhoAmbXZ.dat');

%-------------------------------------------------------------------------%
% Derive main parameters %
%-------------------------------------------------------------------------%
Nb.Links = size(Links.data);
Nb.Links = Nb.Links(1);
Nb.Points = Nb.Links+1;
Nb.ChargeLayers = size(ChargeLayers.data);
Nb.ChargeLayers = Nb.ChargeLayers(1);

N.x = Nxyz(1);
N.y = Nxyz(2);
N.z = Nxyz(3);

d.x = dxyz(1); %_km
d.y = dxyz(2); %_km
d.z = dxyz(3); %_km

Init.z = InitPoint(3);
L.x = (N.x-1)*d.x; %_km
L.y = (N.y-1)*d.y; %_km
L.z = (N.z-1)*d.z; %_km
clear Nxyz dxyz InitPoint

ChargeLayers.Type = 'disks' ; %'none' ; % 'spheres'; %
ChargeLayers.Line.Style = '-';
ChargeLayers.Line.Width = 1;
ChargeLayers.FontSize = 8;
ChargeLayers.Pos.x = 10;
ChargeLayers.Circles = 12;
ChargeLayers.Edge.Color = [[1 0 0];[0 0 1];[1 0 0];[.75 .75 .75]];
% ChargeLayers.Edge.Color = ['none';'none';'none';'none'];
ChargeLayers.Density = 'on';
Ground.Line.Width = .25;

%-------------------------------------------------------------------------%
% Zoom Area %
%-------------------------------------------------------------------------%
FocusArea.x(1) = 0;
FocusArea.x(2) = L.x;
FocusArea.y(1) = 0;
FocusArea.y(2) = L.y;
FocusArea.z(1) = z_gnd;
FocusArea.z(2) = L.z+z_gnd;

%-------------------------------------------------------------------------%
% Fonts %
%-------------------------------------------------------------------------%
Links.Line.Width = 1;
Links.Scheme = 'R/B';
Links.MarkerSize = 3;
Font.Size.Labels = 12;
Font.Size.Axis = 10;
Font.Name = 'Helvetica';

if strcmp(Links.Scheme,'Colormap')
    Links.Color = colormap(jet(Nb.Points));
elseif strcmp(Links.Scheme,'B/W')
    Links.Color = [0 0 0];
elseif strcmp(Links.Scheme,'R/B')
    Links.Color(1,:) = [0 0 0];
    Links.Color(2,:) = [1 0 0];
    Links.Color(3,:) = [0 0 1];
elseif strcmp(Links.Scheme,'B/R')
    Links.Color(1,:) = [0 0 0];
    Links.Color(2,:) = [0 0 1];
    Links.Color(3,:) = [1 0 0];
end

x = (0:N.x-1)*d.x;
y = (0:N.y-1)*d.y;
z = (0:N.z-1)*d.z+z_gnd;

%% Plot
figure(1);
set(gcf,'Units','normalized','OuterPosition', [0 1/3 1 2/3])

%% Subplot xz %%
subplot(121);
set(gca,'FontSize',Font.Size.Axis,'FontName',Font.Name)
set(gca,'XMinorTick','on','YMinorTick','on')
set(gca,'XAxisLocation','bottom','YAxisLocation','left')
set(gca,'TickDir','in')
set(gca,'TickLength',[.025 .05])
set(gca,'LineWidth',.25)
% set(gca,'YTickLabel','')

hold on
if strcmp(ChargeLayers.Density,'on')
    imagescSgnLog(x,z,rho.XZ,-3,1);
    axis([0 12 z(1)*1e-3 z(end)*1e-3])
    xlabel('x (km)','fontsize',18)
    ylabel('z (km)','fontsize',18)
    XTick = get(gca,'XTick');
    set(gca,'XMinorTick','on','YMinorTick','on','XTick',[0 6 12 18]);
    title('\rho_t=\rho_s+\rho_f (nC/m^3)','fontsize',18);
    set(gca,'FontSize',18);
end
plot(...
    Links.data(1,1)*d.x,(Links.data(1,3)*d.z+z_gnd),...
    'x-','LineWidth',Links.Line.Width/5,'MarkerSize',Links.MarkerSize,...
    'MarkerEdgeColor',Links.Color(1,:),'MarkerFaceColor',Links.Color(1,:));

for ii=1:Nb.Links
    % plots ending point of each link %
    if strcmp(Links.Scheme,'Colormap')
        plot(...
            [Links.data(ii,1)*d.x, Links.data(ii,4)*d.x],...
            [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
            'Color',Links.Color(ii,:)...
            );
    elseif strcmp(Links.Scheme,'B/W')
        plot(...
            [Links.data(ii,1)*d.x, Links.data(ii,4)*d.x],...
            [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
            'Color','k'...
            );
    elseif strcmp(Links.Scheme,'R/B') || strcmp(Links.Scheme,'B/R')
        if(Links.data(ii,6)*d.z >= Init.z)
            plot(...
                [Links.data(ii,1)*d.x, Links.data(ii,4)*d.x],...
                [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
                'Color',Links.Color(2,:)...
                );
        else
            plot(...
                [Links.data(ii,1)*d.x, Links.data(ii,4)*d.x],...
                [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
                'Color',Links.Color(3,:)...
                );
        end
    end
end
plot([FocusArea.x(1) FocusArea.x(2)], [z_gnd, z_gnd], 'k', 'LineWidth', Ground.Line.Width);
hold off

box on
axis image
axis([FocusArea.x(1) FocusArea.x(2) FocusArea.z(1) FocusArea.z(2)]);

%% Subplot yz %%
subplot(122);
set(gca,'FontSize',Font.Size.Axis,'FontName',Font.Name)
set(gca,'XMinorTick','on','YMinorTick','on')
set(gca,'XAxisLocation','bottom','YAxisLocation','left')
set(gca,'TickDir','in')
set(gca,'TickLength',[.025 .05])
set(gca,'LineWidth',.25)
% set(gca,'YTickLabel','')

hold on
if strcmp(ChargeLayers.Density,'on')
    imagescSgnLog(y,z,rho.YZ,-3,1);
    axis([y(1) y(end) z(1)*1e-3 z(end)*1e-3])
    xlabel('y (km)','fontsize',18)
    ylabel('z (km)','fontsize',18)
    XTick = get(gca,'XTick');
    set(gca,'XMinorTick','on','YMinorTick','on','XTick',[0 6 12 18]);
    title('\rho_t=\rho_s+\rho_f (nC/m^3)','fontsize',18);
    set(gca,'FontSize',18);
end
plot(...
    Links.data(1,2)*d.y,(Links.data(1,3)*d.z+z_gnd),...
    'x-','LineWidth',Links.Line.Width/5,'MarkerSize',Links.MarkerSize,...
    'MarkerEdgeColor',Links.Color(1,:),'MarkerFaceColor',Links.Color(1,:));

for ii=1:Nb.Links
    % plots ending point of each link %
    if strcmp(Links.Scheme,'Colormap')
        plot(...
            [Links.data(ii,2)*d.y, Links.data(ii,5)*d.y],...
            [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
            'Color',Links.Color(ii,:)...
            );
    elseif strcmp(Links.Scheme,'B/W')
        plot(...
            [Links.data(ii,2)*d.y, Links.data(ii,5)*d.y],...
            [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
            'Color','k'...
            );
    elseif strcmp(Links.Scheme,'R/B') || strcmp(Links.Scheme,'B/R')
        if(Links.data(ii,6)*d.z >= Init.z)
            plot(...
                [Links.data(ii,2)*d.y, Links.data(ii,5)*d.y],...
                [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
                'Color',Links.Color(2,:)...
                );
        else
            plot(...
                [Links.data(ii,2)*d.y, Links.data(ii,5)*d.y],...
                [Links.data(ii,3)*d.z+z_gnd, Links.data(ii,6)*d.z+z_gnd],...
                'Color',Links.Color(3,:)...
                );
        end
    end
end
plot([FocusArea.y(1) FocusArea.y(2)], [z_gnd, z_gnd], 'k', 'LineWidth', Ground.Line.Width);
hold off

box on
axis image
axis([FocusArea.y(1) FocusArea.y(2) FocusArea.z(1) FocusArea.z(2)]);

%% Ambient charge density and tree
% cd Figures
hgexport(gcf,'~/Desktop/BJ/Fractal.ai');
% cd ..