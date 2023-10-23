# ExplodeGrenade
A throwable grenade provide with counter, explosion and path prediction. The grenade can be picked up, move around and cause explosion after counter reach zero.

 To make it works you have to set up followings :
- Add this component to Default Pawn
- Set up Search Scale as the length between player and pickable objects
- Add an explosion particle to the particle slot as it will be trigger after counter reach zero
- Use Throw Strength to set up the strength of throwing grenade. The higher the value is, the further the grenade fly
- Set up Count Down Second to decide when should the counter count down from
- Call the Action Grab and Action Release node in the blueprint and connect to and keyboard action. We can use the keyboard action to trigger the pick up event
- Ready to go


 ![ExplodeGrenade](https://github.com/TimChen1383/ExplodeGrenade_UE/assets/37008451/e7d5a2a7-3435-4475-bf9f-8e8309649f9b)

