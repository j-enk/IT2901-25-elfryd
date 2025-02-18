// ***********************************************
// This example commands.js shows you how to
// create various custom commands and overwrite
// existing commands.
//
// For more comprehensive examples of custom
// commands please read more here:
// https://on.cypress.io/custom-commands
// ***********************************************
//
//
// -- This is a parent command --
// Cypress.Commands.add('login', (email, password) => { ... })
//
//
// -- This is a child command --
// Cypress.Commands.add('drag', { prevSubject: 'element'}, (subject, options) => { ... })
//
//
// -- This is a dual command --
// Cypress.Commands.add('dismiss', { prevSubject: 'optional'}, (subject, options) => { ... })
//
//
// -- This will overwrite an existing command --
// Cypress.Commands.overwrite('visit', (originalFn, url, options) => { ... })

//Command retrieved from https://docs.cypress.io/guides/end-to-end-testing/google-authentication

Cypress.Commands.add('login', () => {
    cy.log('Obtaining access token')
    cy.setCookie("refresh_token", Cypress.env('refreshToken'))
    cy.request({
      method: 'POST',
      url: "https://localhost:5001/api/Auth/refresh-token",
      headers: {
        Authorization: `Bearer ${Cypress.env('accessToken')}`
      }
    })
   .then(({}) => {
      cy.visit('http://localhost:3000/booking')
      })
    })

Cypress.Commands.add('adminLogin', () => {
    cy.log('Obtaining access token')
    cy.setCookie("refresh_token", Cypress.env('refreshTokenAdmin'))
    cy.request({
      method: 'POST',
      url: "https://localhost:5001/api/Auth/refresh-token",
      headers: {
        Authorization: `Bearer ${Cypress.env('accessTokenAdmin')}`
      }
    })
   .then(({}) => {
      cy.visit('http://localhost:3000/admin')
      })
    })