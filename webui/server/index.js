process.env.DB_URI = process.env.DB_URI || 'mongodb://localhost/nextepc';

const next = require('next');

const dev = process.env.NODE_ENV !== 'production';
const app = next({ dev });
const handle = app.getRequestHandler();

const express = require('express');
const bodyParser = require('body-parser');
const morgan = require('morgan');
const session = require('express-session');

const mongoose = require('mongoose');
const MongoStore = require('connect-mongo')(session);

const passport = require('passport');
const LocalStrategy = require('passport-local').Strategy;
const csrf = require('lusca').csrf();
const secret = process.env.SECRET_KEY || 'change-me';

const api = require('./routes');

const Account = require('./models/account.js');

mongoose.Promise = global.Promise;
const db = mongoose.connection;

if (dev) {
  mongoose.set('debug', true);
}

mongoose.connect(process.env.DB_URI)
.then(() => {
  return app.prepare();
})
.then(() => {
  // FIXME : we need to implement landing page for inserting admin account
  Account.findByUsername('admin', true, (err, account) => {
    if (err) {
      console.error(err);
      throw err;
    }
    if (!account) {
      const newAccount = new Account();
      newAccount.username = 'admin';
      newAccount.roles = [ 'admin' ];
      Account.register(newAccount, '1423', err => {
        if (err) {
          console.error(err);
          throw err;
        }
      })
    }
  })

  const server = express();
  
  server.use(bodyParser.json());
  server.use(bodyParser.urlencoded({ extended: true }));

  server.use(session({
    secret: secret,
    store: new MongoStore({ 
      mongooseConnection: mongoose.connection,
      ttl: 60 * 60 * 24 * 7 * 2
    }),
    resave: false,
    rolling: true,
    saveUninitialized: true,
    httpOnly: true,
    cookie: {
      maxAge: 1000 * 60 * 60 * 24 * 7 * 2  // 2 weeks
    }
  }));

  server.use((req, res, next) => {
    csrf(req, res, next);
  })

  server.use(passport.initialize());
  server.use(passport.session());

  passport.use(new LocalStrategy(Account.authenticate()));
  passport.serializeUser(Account.serializeUser());
  passport.deserializeUser(Account.deserializeUser());

  server.use('/api', api);

  server.get('*', (req, res) => {
    return handle(req, res);
  });

  if (dev) {
    server.use(morgan('tiny'));
  }

  server.listen(3000, err => {
    if (err) throw err;
    console.log('> Ready on http://localhost:3000');
  });
})
.catch(err => console.log(err));