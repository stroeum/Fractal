% Initiate
close all
clearvars -except sims
clf

if ~exist('sims','var') || ~isfield(sims,'pathPNGs') || ~isfield(sims,'pathVideos')
    prompt1 = "\nWhat is the planetary body that the simulation is focused on? (No quotation marks needed for string input)\n-->";
    sims.objectName = input(prompt1,'s');
    prompt2 = "\nWhat type of discharge is this? (Leader / Streamer)\n-->";
    sims.objectType = input(prompt2,'s');
    while ~strcmp(sims.objectType,'Streamer') && ~strcmp(sims.objectType,'Leader')
        fprintf('\tNot an acceptable input. Please enter Streamer or Leader.\n');
        sims.objectType = input(prompt2,'s');
    end

    % Settings to ensure proper directory referencing:
    sims.pathPNGs = ['../Figures/',sims.objectName,'/',sims.objectType,'/PNGs'];
    if ~exist(sims.pathPNGs,'dir')
        mkdir(sims.pathPNGs);
    end
    sims.pathVideos = ['../Figures/',sims.objectName,'/',sims.objectType,'/Videos'];
    if ~exist(sims.pathVideos,'dir')
        mkdir(sims.pathVideos);
    end
end 

cd ../results

% Load data files
load('dxyz.dat',             '-ascii');
load('Nxyz.dat',             '-ascii');
load('InitPoint.dat',        '-ascii');
rho.data = load('rhoAmb.dat',           '-ascii');
Links.ID = load('EstablishedLinks.dat', '-ascii');
gnd.alt  = load('z_gnd.dat',            '-ascii');

if isempty(Links.ID)
    fprintf('\n*** LightningVisual.m cannot be executed with current EstablishedLinks.dat file. ***\n');
    cd ../viz
    return
else
    fprintf('\n*** Executing LightningVisual.m script. ***\n');
    % User-Based Settings:
    prompt_Rec = '\nWould you like to record the lightning propagation as a movie? (Y / N)\n-->';
    is.Rec = input(prompt_Rec,'s');                    
    while ~strcmp(is.Rec,'Y') && ~strcmp(is.Rec,'N')
        fprintf('\n\tNot an acceptable input. Please enter Y (for yes) or N (for no).\n');
        is.Rec = input(prompt_Rec,'s');
    end
    prompt_updateChargeDensity = '\nWould you like to update the charge distribution coloring for every saved timestep? (Y / N)\n-->';
    is.updateChargeDensity = input(prompt_updateChargeDensity,'s');                    
    while ~strcmp(is.updateChargeDensity,'Y') && ~strcmp(is.updateChargeDensity,'N')
        fprintf('\n\tNot an acceptable input. Please enter Y (for yes) or N (for no).\n');
        is.updateChargeDensity = input(prompt_updateChargeDensity,'s');
    end
    prompt_highResolution = '\nWould you like to save the final image as a very high resolution image? (Y / N)\nWARNING: Only recommended for preparing posters.\n-->';
    is.highResolution = input(prompt_highResolution,'s');                    
    while ~strcmp(is.highResolution,'Y') && ~strcmp(is.highResolution,'N')
        fprintf('\n\tNot an acceptable input. Please enter Y (for yes) or N (for no).\n');
        is.highResolution = input(prompt_highResolution,'s');
    end
end

% Derive main parameters
Links.Nb = size(Links.ID);
Links.Nb = Links.Nb(1);
N.x = Nxyz(1);
N.y = Nxyz(2);
N.z = Nxyz(3);

d.x = dxyz(1);           % _m
d.y = dxyz(2);           % _m
d.z = dxyz(3);           % _m

L.x = (N.x-1)*d.x;         % _m
L.y = (N.y-1)*d.y;         % _m
L.z = (N.z-1)*d.z;         % _m

S.x = InitPoint(1);   % _m
S.y = InitPoint(2);   % _m
S.z = InitPoint(3);   % _m
S.R = InitPoint(4);   % _m

S.i = round(S.x/d.x);
S.j = round(S.y/d.y);
S.k = round(S.z/d.z);

rho.data = ConvertTo3d(rho.data,Nxyz); % _C/_m^3

clear dxyz
clear InitPoint
cd ../viz

% Linear spaces for the three position dimensions:
x = ((0:(N.x-1))*d.x)*1e-3;
y = ((0:(N.y-1))*d.y)*1e-3;
z = ((0:(N.z-1))*d.z + gnd.alt)*1e-3;

[X,Y,Z]=meshgrid(x,y,z);
rho.max = .95* max(max(max(rho.data)));
rho.min = .95* min(min(min(rho.data)));

% Map ColorScale
gnd.color = [.75 .75 .75];%[.718 .255 .055];
color     = colormap(jet(Links.Nb));
is.BW = 1; %input('Is plot Monochrome? (1: yes, else: no)\n>> ');
if (is.BW == 1)
    for ii=1:Links.Nb
        color(ii,:) = [0 0 0];
    end
end
% Set movie recording
if (strcmp(is.Rec,'Y') == 1)
    Movie = VideoWriter([sims.pathVideos,'/',sims.objectName,'_',sims.objectType,'Video'],'MPEG-4');
    open(Movie);
end
% Draw the tree
figure(1);
set(gcf,'Position',[0,0,800,1000]);
set(gcf,'Resize','off')
hold on;
grid on;

% Sets bounds for the axes (comment out if clouds get cut off):
axis([L.x*1/5 L.x*4/5 L.y*1/5 L.y*4/5 gnd.alt 2/2*(L.z+gnd.alt)]*1e-3) % Slight crop
%axis([0 L.x 0 L.y gnd.alt 2/2*(L.z+gnd.alt)]*1e-3)                     % Full span 

% Plots the cloud structure with the defined function below:
axis equal
xlim([0 max(x)]);
ylim([0 max(y)]);
%zlim([25 70]); % to crop the altitude range for visualization
zlim([0 max(z)]);
set(legend,'Position',[0.225 0.7 .5 .0375],'box','off')
%set(legend,'location','southoutside','box','on')
set(gcf,'Resize','off')

% Represents the neutrally charged (grounded) surface:
P.x = [L.x 0 0 L.x]*1e-3;
P.y = [L.y L.y 0 0]*1e-3;
P.z = [gnd.alt gnd.alt gnd.alt gnd.alt]*1e-3;
patch(P.x, P.y, P.z, gnd.alt,'FaceColor',gnd.color,'HandleVisibility','off');

% Initialize distance traveled for lightning links:
distance = 0;
% For-loop to plot lightning discharge links:
for ii=1:Links.Nb
    %     if(rem(ii,10)==0)
    %         %         pause;
    %     end
    if mod((ii-1),100) == 0
        if ii == 1
            plottingChargeRegions('white',0.25,rho,X,Y,Z);
        else
            if strcmp(is.updateChargeDensity,'Y') == 1
                allPatches = findall(gcf,'type','patch');
                delete(allPatches);
                rho.data = load(['../results/rho3d',num2str(ii-1),'.dat'],           '-ascii');
                rho.data = ConvertTo3d(rho.data,Nxyz); % _C/_m^3
                rho.max = .95* max(max(max(rho.data)));
                rho.min = .95* min(min(min(rho.data)));
                plottingChargeRegions('white',0.4,rho,X,Y,Z);
            end
        end
    end    
    % Defining initial position of lightning link (m)
    x1 = Links.ID(ii,1)*d.x;
    y1 = Links.ID(ii,2)*d.y;
    z1 = Links.ID(ii,3)*d.z+gnd.alt; 

    % Defining final position of lightning link (m)
    x2 = Links.ID(ii,4)*d.x;
    y2 = Links.ID(ii,5)*d.y;
    z2 = Links.ID(ii,6)*d.z+gnd.alt;
    
    % Summing lightning link to overall distance of link traveled (m):
    distance = distance + sqrt(((x2-x1)^2) + ((y2-y1)^2) + ((z2-z1)^2));

    % Plotting link:
    plot3(...
        [x1, x2]*1e-3,...
        [y1, y2]*1e-3,...
        [z1, z2]*1e-3,...
        'Color',color(ii,:),'HandleVisibility','off');
    
    % Formatting axes:
    set(gcf,'Position',[0,0,800,1000]);
    set(gcf,'Resize','off')
    %axis equal
    %{
    xticks([0 5 10 15 20]);
    xticklabels({'0','5','10','12'});
    yticks([0 4 8 12]);
    yticklabels({'0','4','8','12'});
    zticks([46 50 54 58 62 66 70]);
    zticklabels({'46','50','54','58','62','66','70'});
    %}
    box on
    title([sims.objectType,' discharge after ', int2str(ii) ,' step(s)'],'FontSize',28,'FontWeight','bold','Interpreter','latex');
    if(strcmp(is.Rec,'Y') == 1)
        set(gcf,'Position',[0,0,800,1000]); 
        set(gcf,'Resize','off')
        frame = getframe(gcf);
        writeVideo(Movie,frame);
    end
end
fprintf(['\n\t',sims.objectType,' has propagated %.2f meters\n',distance]);
pause
%camlight; lighting gouraud


hold off;
% Record the movie
if (strcmp(is.Rec,'Y') == 1)
    frame = getframe(gcf);
    writeVideo(Movie,frame);
    close(Movie);
end
title(['Simulated ',sims.objectType,' Discharge: ',sims.objectName],'FontSize',28,'FontWeight','bold','Interpreter','latex');
set(gcf,'Position',[0,0,800,1000]); 
set(gcf,'Resize','off')
% If the 'export_fig' function is assigned to the pathtool:
if exist('export_fig') == 2 && strcmp(is.highResolution,'Y') == 1
    export_fig ../Figures/HighRes_Discharge.png -transparent -m8
else
    exportgraphics(gcf,[sims.pathPNGs,'/Lightning_',sims.objectName,'_',sims.objectType,'.png'],'Resolution',600);
end
    
function [AA] = ConvertTo3d(A,B)
    [M, N] = size(A);
    AA = zeros(B');
    for m=1:M
        for n=1:N
            ii = rem(m,B(1));
            if(ii==0)
                ii = B(1);
            end
            jj = n;
            kk = (m-ii)/B(1)+1;
            AA(ii,jj,kk) = A(m,n);
        end
    end
end