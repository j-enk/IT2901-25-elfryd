describe("Test booking admin functionality", () => {
  beforeEach(() => {
    const now = new Date(2025, 4, 1).getTime();
    cy.clock(now);
  });

  before(() => {
    const now = new Date(2025, 4, 1).getTime();
    cy.clock(now);

    // Create a booking period
    cy.adminLogin();
    cy.tick(1000);
    cy.contains("Legg til ny bookingperiode").click();
    cy.get("#mui-5").type("Test Bookingperiode");
    cy.get('[name="StartDate"]').type("2025-05-01");
    cy.get('[name="EndDate"]').type("2025-06-01");
    cy.contains("Velg åpningstid").next().clear().type("7");
    cy.contains("Velg stengetid").next().clear().type("22");
    cy.contains("Legg til bookingperiode").click();

    // Create booking
    cy.login();
    cy.tick(1000);
    cy.get(":nth-child(7) > .fc-timegrid-slot-lane").click();
    cy.get(".MuiButton-root").click();
  });

  it("Test admin can edit booking", () => {
    cy.adminLogin();
    cy.visit("http://localhost:3000/booking");
    cy.tick(1000);
    cy.contains("9:00").should("not.exist");
    cy.get(
      '[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main'
    ).click();
    cy.contains("Test Testesen").should("exist");
    cy.contains("Rediger booking").should("exist").click();
    // Edits booking and checks that it is now from 0900-1030
    cy.get(":nth-child(2) > .MuiInput-input").type("2025-05-01T09:00");
    cy.contains("Lagre booking").should("exist").click();
    cy.contains("9:00").should("exist");
  });

  it("Test admin can delete booking", () => {
    cy.adminLogin();
    cy.visit("http://localhost:3000/booking");
    cy.tick(1000);
    cy.get(
      '[style="inset: 100px 0% -175px; z-index: 1;"] > .fc-timegrid-event'
    ).click();
    cy.contains("Kanseller booking").click();
    cy.contains("Ja, jeg er sikker").click();
    cy.get(
      '[style="inset: 100px 0% -175px; z-index: 1;"] > .fc-timegrid-event'
    ).should("not.exist");
  });

  it("Test admin can create maintenance period", () => {
    cy.adminLogin();
    cy.visit("http://localhost:3000/booking");
    cy.tick(1000);
    // Create maintenance period
    cy.get(":nth-child(7) > .fc-timegrid-slot-lane").click();
    cy.contains("Vedlikehold").should("exist").click();
    cy.get("#mui-5").type("Vedlikehold av Elfryd");
    cy.get(".MuiButton-root").click();
    cy.reload();
    cy.tick(1000);
    // Edit maintenance period
    cy.get(
      '[style="inset: 150px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main'
    ).click();
    cy.contains("Rediger booking").click();
    cy.get(":nth-child(2) > .MuiInput-input").type("2025-05-01T09:00");
    cy.contains("Lagre booking").click();
    cy.reload();
    cy.tick(1000);
    cy.contains("9:00").should("exist");

    // Check that maintenance period exists, and deletes it
    cy.contains("Vedlikehold av Elfryd").should("exist");
    cy.get(
      '[style="inset: 100px 0% -175px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main'
    ).click();
    cy.contains("Kanseller booking").click();
    cy.get(
      '[style="inset: 100px 0% -225px; z-index: 1;"] > .fc-timegrid-event > .fc-event-main'
    ).should("not.exist");
  });

  after(() => {
    // Delete booking period
    cy.adminLogin();
    cy.tick(1000);
    cy.contains("Bookingperioder").click();
    cy.contains("Test Bookingperiode")
      .parent()
      .within(() => {
        cy.get(".MuiSvgIcon-root").parent().click();
      });
    cy.contains("Ja, jeg er sikker").click();
  });
});
export {};