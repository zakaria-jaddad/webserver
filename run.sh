# Build the image
# docker build -t nginx-2010 .

# Run the container, mapping port 2010
# docker run -d -p 2010:2010 nginx-2010
#
make all
./ft_webserver conf/test.conf
