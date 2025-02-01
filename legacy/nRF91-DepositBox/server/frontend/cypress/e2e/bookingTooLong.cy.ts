describe('BookingOutsidePeriod', function () {
    it('Tries to create booking outside booking period', function () {
        //Finds a time slot outside booking periods and checks that it is unable to create booking there
        cy.login()
        cy.clock(Date.UTC(2022, 7, 22), ['Date'])
        cy.get(':nth-child(7) > .fc-timegrid-slot-lane').click()
        cy.get(':nth-child(2) > .MuiInput-input').type("2022-08-26T10:00")
        cy.get('.MuiButton-root').click()
        cy.get('.MuiAlert-message').contains("Booking is too long.")
        cy.get('body').type("{esc}")

        //Check that booking in same time slot is possible as previous booking was not made
        cy.get(':nth-child(7) > .fc-timegrid-slot-lane').click()
        cy.get('#headlessui-dialog-title-12').contains("Book båten")
    })
})

export {};