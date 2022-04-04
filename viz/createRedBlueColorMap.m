% % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %
% File Name: createRedBlueColorMap.m                                      %
% Purpose: Creates a custom color map for negative charge densities (blue)%
%          and positive charge densities (red). Assumed preference for    %
%          colorblind-friendly mode, 'white'.                             %
% Author: Annelisa Esparza                                                %
% Contact: aesparza2014@my.fit.edu                                        %
% Added Date: February 22, 2022                                           %
% Last Update: April 4, 2022 - Updated to account for an alpha value.     %
% % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %

function customColorMap = createRedBlueColorMap(neutralcolor,alphavalue)
    customColorMap = zeros([101,3]);
    customColorMap(51,:) = [0.75 0.75 0.75];
    if strcmp(neutralcolor,'white') == 1 || strcmp(neutralcolor,'White') == 1
        % Version 1: blue to white to red
        increasing_value = linspace((1-alphavalue),1,50);
        decreasing_value = linspace(1,(1-alphavalue),50);
        steady_value = ones([50,1]);
        customColorMap(1:50,1) = increasing_value;
        customColorMap(1:50,2) = increasing_value;
        customColorMap(1:50,3) = steady_value;
        customColorMap(52:end,1) = steady_value;
        customColorMap(52:end,2) = decreasing_value;
        customColorMap(52:end,3) = decreasing_value;
    else
        % Version 2: blue to grey to red
        increasing4zero_value = linspace(0,0.75,50);
        decreasing4zero_value = linspace(0.75,0,50);
        increasing4one_value = linspace(0.75,1,50);
        decreasing4one_value = linspace(1,0.75,50);
        customColorMap(1:50,1) = increasing4zero_value;
        customColorMap(1:50,2) = increasing4zero_value;
        customColorMap(1:50,3) = decreasing4one_value;
        customColorMap(52:end,1) = increasing4one_value;
        customColorMap(52:end,2) = decreasing4zero_value;
        customColorMap(52:end,3) = decreasing4zero_value;
    end
end