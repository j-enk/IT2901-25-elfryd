describe("Test booking period administration functionality", () => {
  it("Test adding booking period", () => {
    cy.adminLogin();
    cy.contains("Legg til ny bookingperiode").click();

    // Check that empty form cannot be submitted
    cy.contains("Legg til bookingperiode").click();
    cy.get("input:invalid").should("exist");

    // Fill in form with end date before start date
    cy.get("#mui-5").type("Test Bookingperiode");
    cy.get('[name="StartDate"]').type("2025-12-01");
    cy.get('[name="EndDate"]').type("2025-06-01");
    cy.contains("Velg åpningstid").next().type("7");
    cy.contains("Velg stengetid").next().type("22");
    cy.contains("Legg til bookingperiode").click();
    cy.contains("Start date must be earlier than end date.").should("exist");

    // Fill in form with booking closes before booking opens
    cy.get("#mui-5").type("Test Bookingperiode");
    cy.get('[name="StartDate"]').type("2025-05-01");
    cy.get('[name="EndDate"]').type("2025-06-01");
    cy.contains("Velg åpningstid").next().type("22");
    cy.contains("Velg stengetid").next().type("7");
    cy.contains("Legg til bookingperiode").click();
    cy.contains("Opening time must be earlier than closing time.").should(
      "exist"
    );

    // Correct input and submit form
    cy.get("#mui-5").type("Test Bookingperiode");
    cy.get('[name="StartDate"]').type("2025-05-01");
    cy.get('[name="EndDate"]').type("2025-06-01");
    cy.contains("Velg åpningstid").next().clear().type("7");
    cy.contains("Velg stengetid").next().clear().type("22");
    cy.contains("Legg til bookingperiode").click();

    cy.contains("Bookingperioden ble lagt til").should("exist");
  });

  it("Test created booking period is as intended", () => {
    const now = new Date(2025, 4, 1).getTime();
    cy.clock(now);
    cy.adminLogin();
    cy.tick(1000);
    // Checks that booking period is in booking period list
    cy.contains("Bookingperioder").click();
    cy.contains("Test Bookingperiode").should("exist");
    // Checks that calendar is interactable inside of booking period
    cy.visit("http://localhost:3000/booking");
    cy.tick(1000);
    cy.get(":nth-child(2) > .fc-timegrid-slot-lane").click();
    cy.tick(1000);
    cy.contains("Book båten").should("exist");
    cy.reload();
    cy.tick(1000);
    cy.get(".fc-prev-button").click();
    // Checks that calendar is not interactable outside of booking period
    cy.get(":nth-child(2) > .fc-timegrid-slot-lane").click();
    cy.tick(1000);
    cy.contains("Book båten").should("not.exist");
  });

  it("Test delete booking period", () => {
    cy.adminLogin();
    cy.contains("Bookingperioder").click();
    cy.contains("Test Bookingperiode")
      .parent()
      .within(() => {
        cy.get(".MuiSvgIcon-root").parent().click();
      });
    cy.contains("Ja, jeg er sikker").click();
    cy.wait(500);
    cy.contains("Test Bookingperiode").should("not.exist");
  });
});
export {};