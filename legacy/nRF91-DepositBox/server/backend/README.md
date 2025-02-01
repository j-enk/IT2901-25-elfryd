# Backend

The backend is written in C# using the .NET framework. It is a REST API, and has a MQTT client. It has logic for handling boats, bookings, booking periods, lock boxes, and authentication.

If changes are made to the models then `depositbox.sqlite` has to be deleted. And migrations re-ran with `dotnet ef database update --context LockBoxContext`

## Code structure
    backend.API/
    ├─────── Controllers/
    |        ├── AuthController
    |        ├── BoatsController
    |        ├── BookingPeriodsController
    |        ├── BookingsController
    |        ├── LockBoxesController
    |        ├── UsersController
    ├─────── Data/
    |        ├── Models/
    |        ├── Services/
    |        ├── ViewModels/
    |        ├── AppDbInitializer
    |        ├── LockBoxContext/
    ├─────── MQTT
    ├─────── Handlers
