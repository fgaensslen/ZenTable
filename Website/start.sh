#!/bin/bash
export NVM_DIR="/home/admin/.nvm"
source "$NVM_DIR/nvm.sh"
nvm use 20
cd /home/admin/ZenTable/Website
export NODE_ENV=production
export NODE_OPTIONS="--openssl-legacy-provider --no-deprecation"
/home/admin/ZenTable/Website/node_modules/.bin/nodemon server/app.js --exec /home/admin/ZenTable/Website/node_modules/.bin/babel-node --ignore 'files/*' --ignore 'playlists.json'
