close all;
clear all;

data = csvread('/tmp/measurements.csv');


N=size(unique(data(:,2)),1);

fft_size=size(data,2)-3;

spectrum = [];

for i=1:N:(size(data,1)-N)
    c = data(i:i+N-1,3:end-1);
    b=reshape(c',N*fft_size,1);
    spectrum = [spectrum; b'];
end

imagesc(spectrum);

