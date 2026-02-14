# Use the official Nginx base image
FROM nginx:latest

# Expose port 2010
EXPOSE 2010

# Replace the default Nginx configuration to listen on port 2010
RUN sed -i 's/listen\s*80;/listen 2010;/g' /etc/nginx/conf.d/default.conf && \
    sed -i 's/listen\s*\[::\]:80;/listen [::]:2010;/g' /etc/nginx/conf.d/default.conf

# Start Nginx in the foreground
CMD ["nginx", "-g", "daemon off;"]
