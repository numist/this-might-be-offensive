FROM node:14-alpine

WORKDIR /app

# Copy package.json for dependency installation
COPY realtime/package.json ./

# Install dependencies with legacy peer deps for old socket.io
RUN npm install --legacy-peer-deps

# Copy the application code
COPY realtime/ ./

# Create admin directory for config file mount
RUN mkdir -p /app/admin

EXPOSE 1337

ENV NODE_ENV=production

CMD ["node", "app.js"]
