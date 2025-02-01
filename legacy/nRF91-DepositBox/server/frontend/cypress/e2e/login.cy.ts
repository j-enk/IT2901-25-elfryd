describe("Login", function () {

  it("Checks that calender exists when logged in", function () {
    cy.login();
    cy.get(".fc").should("exist");
  });

  it("Checks that login page shows when not logged in", function () {
    cy.visit('http://localhost:3000/booking')
    cy.get('[alt="Kystlaget Trondheim logo"]').should("exist")
  });

});

export { };
