describe('Booking', function () {
  beforeEach(function () {
    //Sets the date to a time slot within a booking period, where no previous booking exists
    cy.clock(Date.UTC(2022, 7, 14), ['Date'])
    cy.login()
  })

  it('Creates booking and then deletes it', function () {
    //Create booking
    cy.get(':nth-child(7) > .fc-timegrid-slot-lane').click()
    cy.get('.MuiButton-root').click()
    cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').should("exist")
    //Delete booking that was just created
    cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').click()
    cy.get('.bg-red-700').click()
    cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').should("not.exist")
  })

  it('Creates booking and then edits it', function () {
    //Create booking
    cy.get(':nth-child(7) > .fc-timegrid-slot-lane').click()
    cy.get('.MuiButton-root').click()
    cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').should("exist")

    //Edit booking that was just created
    cy.get('[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-time').click()
    cy.get('.MuiDialogActions-root > :nth-child(2)').click()
    cy.get(':nth-child(2) > .MuiInput-input').type("2022-08-11T10:00")
    cy.get('.MuiButton-outlinedSuccess').click()

    //check if booking is changed 
    cy.get('[style="inset: 100px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main > .fc-event-main-frame > .fc-event-title-container > .fc-event-title').click()
    cy.get('.flex-row > :nth-child(1) > :nth-child(2)').contains("10:00")

    //delete booking after test has completed
    cy.get('.bg-red-700').click()
  })
})
export {};