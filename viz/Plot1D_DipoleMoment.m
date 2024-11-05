close all
clearvars -except sims


if ~exist('sims','var') || ~isfield(sims,'pathPNGs') || ~isfield(sims,'pathVideos')
    prompt1 = "\nWhat is the planetary body that the simulation is focused on? (No quotation marks needed for string input)\n-->";
    sims.objectName = input(prompt1,'s');
    prompt2 = "\nWhat type of discharge is this? (Leader / Streamer)\n-->";
    sims.objectType = input(prompt2,'s');
    while ~strcmp(sims.objectType,'Streamer') && ~strcmp(sims.objectType,'Leader')
        fprintf('\n\tNot an acceptable input. Please enter Streamer or Leader.\n');
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

    % Specifies the boundary conditions for the simulation:
    prompt_BCtype = '\nIs the domain in free space (FS) or is z = 0 grounded (G)?\n-->';
    sims.BCtype = input(prompt_BCtype,'s');                    
    while ~strcmp(sims.BCtype,'FS') && ~strcmp(sims.BCtype,'G')
        fprintf('\n\tNot an acceptable input. Please enter FS (for free space) or G (for grounded).\n');
        sims.BCtype = input(prompt_BCtype,'s');
    end
end 

cd ../results/
p = load('DischargeDipoleMoment.dat')*1e-3; % convert to C/km
if isempty(p) || (size(p,1)-1) == 0
    fprintf('\n*** Plot1D_DipoleMoment.m cannot be executed with current DischargeDipoleMoment.dat file. ***\n');
    cd ../viz
    return
else
    fprintf('\n*** Executing Plot1D_DipoleMoment.m script. ***\n');
end
NbOfPoints  = size(p);
NbOfPoints  = NbOfPoints(1);
step        = (0:NbOfPoints-1)';
VectorStep  = 25;                   % plot p vector every VectorStep step.
 
figure;
set(gcf,'Units','inches','OuterPosition', [20 20 20 40]/6)
% set(gca,'Units','inches','Position', [4 3 15 11]/6)

subplot(211)
absp = (p(:,1).^2+p(:,2).^2+p(:,3).^2).^.5;
% hold on
plot(step,absp,'LineWidth',1,'LineStyle','-','color','k');
% plot(step,-p(:,3),'LineWidth',1,'LineStyle','--','color','b');
% hold off
xlabel('step','FontSize',12);
ylabel('$\|\vec{p}\|$ (C.km)','Interpreter','latex','FontSize',12);
set(gca,'FontSize',10);
axis([0 max(step) min(0) max(absp)]) ;
box on
grid on

subplot(212)
hold on
plot(step,p(:,1),'LineWidth',1,'LineStyle','-','Color','r');
plot(step,p(:,2),'LineWidth',1,'LineStyle','-','Color','g');
plot(step,p(:,3),'LineWidth',1,'LineStyle','-','Color','b');
xlabel('step','FontSize',12);
ylabel('p_{x,y,z} (C.km)','FontSize',12);
legend('p_x','p_y','p_z','Location','East');
set(gca,'FontSize',10);
axis([0 max(step) min(min(p)) max(max(p))]);
box on
grid on
hold off
exportgraphics(gcf,[sims.pathPNGs,'/DipoleMoment_',sims.objectName,'_',sims.objectType,'.png'],'BackgroundColor','white','Resolution',300);

cd ../viz/