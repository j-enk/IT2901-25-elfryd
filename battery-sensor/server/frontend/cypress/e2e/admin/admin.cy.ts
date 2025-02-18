describe("Test admin miscellaneous functionality", () => {
    
    it("Test admin functionality as regular user", () => {
        cy.login()
        cy.get(".MuiToolbar-root > :nth-child(2) > .MuiButtonBase-root")
        // Admin should not be available in the dropdown menu
        cy.get(".MuiList-root")
            .should("not.contain", 'Admin');
        // Tries to visit admin page
        cy.visit("http://localhost:3000/admin");
        // Checks that user is redirected to booking
        cy.url()
            .should("eq", "http://localhost:3000/booking");
    })

    it("Test key history", () => {
        cy.adminLogin()
        // Check that key history tab exists
        cy.contains("Nøkkelhistorikk")
            .should("exist")
            .click()
        // Checks that when clicked a dropdown is rendererd
        cy.get('#selectLockBox')
            .should("exist")
        // Checks that a list of key events is rendered (does not check for items)
        cy.get('.css-158hx1q > .MuiList-root')
            .should("exist")
    })

})

export {};