import { QueryClient, QueryClientProvider } from "react-query";
import EditUserForm from "../../../components/admin/userList/forms/EditUserForm";

describe("Sucessfull edit", () => {
  it("submits form with updated user data", () => {
    // Set up test data
    const user = {
      id: "1",
      name: "Test Testesen",
      phone: "123456789",
      email: "test@test.com",
      roles: ["member"],
      isActive: true,
    };

    // Stub the editUser mutation
    cy.intercept("PUT", `${process.env.API_URL_DEV}/User/${user.id}`, {
      statusCode: 200,
      body: {
        success: true,
      },
    }).as("editUserMutation");

    // Stub the editUser mutation
    cy.intercept("PUT", `${process.env.API_URL_DEV}/User/${user.id}`, {
      statusCode: 200,
      body: {
        success: true,
      },
    }).as("editUserMutation");

    const setEdit = cy.stub();
    const queryClient = new QueryClient();

    // Mount the component
    cy.mount(
      <QueryClientProvider client={queryClient}>
        <EditUserForm user={user} edit={true} setEdit={setEdit} />
      </QueryClientProvider>
    );

    // Fill in form fields and submit form
    cy.get('input[name="Name"]').clear().type("Ola Nordmann");
    cy.get('input[name="Phone"]').clear().type("987654321");
    cy.get('input[name="Email"]').clear().type("ola@test.com");
    cy.get('button[type="submit"]').click();

    // Verify that the mutation was called with the correct data
    cy.wait("@editUserMutation").then((interception) => {
      const { name, phone, email } = interception.request.body;
      expect(name).to.equal("Ola Nordmann");
      expect(phone).to.equal("987654321");
      expect(email).to.equal("ola@test.com");
    });
  });
});

describe("Failed edit", () => {
  it("Displays error message when fail", () => {
    // Set up test data
    const user = {
      id: "1",
      name: "Test Testesen",
      phone: "123456789",
      email: "test@test.com",
      roles: ["member"],
      isActive: true,
    };

    // Stub the editUser mutation
    cy.intercept("PUT", `${process.env.API_URL_DEV}/User/${user.id}`, {
      statusCode: 500,
      body: {
        success: false,
      },
    }).as("editUserMutation");

    // Stub the editUser mutation
    cy.intercept("PUT", `${process.env.API_URL_DEV}/User/${user.id}`, {
      statusCode: 500,
      body: {
        success: false,
      },
    }).as("editUserMutation");

    const setEdit = cy.stub();
    const queryClient = new QueryClient();

    // Mount the component
    cy.mount(
      <QueryClientProvider client={queryClient}>
        <EditUserForm user={user} edit={true} setEdit={setEdit} />
      </QueryClientProvider>
    );

    // Fill in form fields and submit form
    cy.get('input[name="Name"]').clear().type("Ola Nordmann");
    cy.get('input[name="Phone"]').clear().type("987654321");
    cy.get('input[name="Email"]').clear().type("ola@test.com");
    cy.get('button[type="submit"]').click();

    cy.get("button").contains("Gå tilbake").should("exist");

    // Verify that the error message is displayed
    cy.wait("@editUserMutation");
    cy.contains("Noe gikk galt under lagringen av brukeren");
  });
});

describe("Check if fields are deactivated", () => {
  it("Fields should not be able to be edited when the fields are deactivated", () => {
    // Set up test data
    const user = {
      id: "1",
      name: "Test Testesen",
      phone: "123456789",
      email: "test@test.com",
      roles: ["member"],
      isActive: true,
    };

    const setEdit = cy.stub();
    const queryClient = new QueryClient();

    // Mount the component
    cy.mount(
      <QueryClientProvider client={queryClient}>
        <EditUserForm user={user} edit={false} setEdit={setEdit} />
      </QueryClientProvider>
    );

    // Fill in form fields and submit form
    cy.get('input[name="Name"]').should("be.disabled");
    cy.get('input[name="Phone"]').should("be.disabled");
    cy.get('input[name="Email"]').should("be.disabled");
    cy.get('input[name="Role"]').should("be.disabled");
    cy.get('button[type="submit"]').should("not.exist");
    cy.get("button").should("not.exist");
  });
});
