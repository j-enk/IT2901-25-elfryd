describe('BookingOutsidePeriod', function () {
    it('Tries to create booking outside booking period', function () {
        //Finds a time slot outside booking periods and checks that it is unable to create booking there
        cy.login()
        cy.clock(Date.UTC(2022, 8, 4), ['Date'])
        cy.get('thead > tr > .fc-day-thu').contains("OBS! Booking stengt")
        cy.get(':nth-child(7) > .fc-timegrid-slot-lane').click()
        cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').should("not.exist")
    })
})
export {};