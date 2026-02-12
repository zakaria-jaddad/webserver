FROM nginx:alpine

# Expose port 8080
EXPOSE 8080

# Update nginx configuration to listen on port 8080 instead of 80
RUN sed -i 's/listen\s*80;/listen 8080;/g' /etc/nginx/conf.d/default.conf

# Start nginx
CMD ["nginx", "-g", "daemon off;"]
