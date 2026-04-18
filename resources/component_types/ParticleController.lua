ParticleController = {
	ps = nil,

	OnUpdate = function(self)
		if Application.GetFrame() == 10 then
			local particle_actor = Actor.Instantiate("particle_actor")
			self.ps = particle_actor:GetComponent("ParticleSystem")
		end

		if Application.GetFrame() == 100 then
			self.ps:Stop()
		end


		if Application.GetFrame() == 130 then
			self.ps:Play()
		end
	end
}

