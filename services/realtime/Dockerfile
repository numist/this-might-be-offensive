# Node 22 LTS. The dependency tree is pure JavaScript (no native bindings), so
# socket.io 0.9 runs unchanged here -- verified end to end: token handshake against
# MySQL, and Redis pmessage fan-out to a connected client.
#
# Note this fixes the *runtime* CVEs only. socket.io/ws/uglify-js/xmlhttprequest in
# package.json still have advisories against them; those follow any Node version and
# need a socket.io 4.x upgrade (a breaking protocol change) to clear.
FROM node:22-alpine

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
